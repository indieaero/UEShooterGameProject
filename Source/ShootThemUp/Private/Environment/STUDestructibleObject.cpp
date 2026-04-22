// Shoot Them Up Game, All Rights Reserved.

#include "Environment/STUDestructibleObject.h"
#include "Field/FieldSystemComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

namespace
{
    constexpr float InvulnerableDamageThreshold = 1e9f;
}

ASTUDestructibleObject::ASTUDestructibleObject()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);
    bAlwaysRelevant = true;

    FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>("FieldSystemComponent");
    FieldSystemComponent->SetupAttachment(RootComponent);
}

void ASTUDestructibleObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTUDestructibleObject, Health);
    DOREPLIFETIME(ASTUDestructibleObject, bIsDestroyed);
    DOREPLIFETIME(ASTUDestructibleObject, DestructionFieldPosition);
}

void ASTUDestructibleObject::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        Health = FMath::Max(1.0f, MaxHealth);
    }

    if (!GeometryCollectionRef)
    {
        GeometryCollectionRef = FindComponentByClass<UGeometryCollectionComponent>();
    }

    if (GeometryCollectionRef)
    {
        // GeometryCollection must replicate fracture state from server to clients.
        GeometryCollectionRef->SetIsReplicated(true);
        // Keep GC inert until our Health logic decides it can break.
        GeometryCollectionRef->SetSimulatePhysics(false);

        for (float& Threshold : GeometryCollectionRef->DamageThreshold)
        {
            Threshold = InvulnerableDamageThreshold;
        }
    }
}

float ASTUDestructibleObject::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) return 0.0f;
    if (!FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f) return 0.0f;

    // Save last hit point for field position (when bUseLastHitPoint is true)
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        LastDamageImpactPoint = static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.ImpactPoint;
    }
    else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
    {
        LastDamageImpactPoint = static_cast<const FRadialDamageEvent&>(DamageEvent).Origin;
    }

    if (bIsDestroyed)
    {
        const FVector ImpactPosition = bUseLastHitPoint && LastDamageImpactPoint != FVector::ZeroVector
            ? LastDamageImpactPoint
            : GetActorLocation();
        const float DamageScale = FMath::Clamp(DamageAmount / FMath::Max(1.0f, MaxHealth), 0.05f, 0.35f);
        QueuePostBreakStrain(ImpactPosition, DamageScale);
        return DamageAmount;
    }

    // Don't call Super - we handle damage ourselves to prevent propagation to Geometry Collection
    Health = FMath::Max(0.0f, Health - DamageAmount);

    CheckAndTriggerDestruction();
    return DamageAmount;
}

void ASTUDestructibleObject::TriggerDestructionAt(FVector Location)
{
    if (!HasAuthority() || bIsDestroyed) return;

    DestructionFieldPosition = FVector_NetQuantize(Location);
    bIsDestroyed = true;
    Health = 0.0f;
    ApplyDestructionField(FVector(DestructionFieldPosition));
}

void ASTUDestructibleObject::CheckAndTriggerDestruction()
{
    if (!HasAuthority() || bIsDestroyed || Health > 0.0f) return;

    DestructionFieldPosition = FVector_NetQuantize(GetDestructionFieldPosition());
    bIsDestroyed = true;
    ApplyDestructionField(FVector(DestructionFieldPosition));
}

FVector ASTUDestructibleObject::GetDestructionFieldPosition() const
{
    if (bUseLastHitPoint && LastDamageImpactPoint != FVector::ZeroVector)
    {
        return LastDamageImpactPoint;
    }
    return GetActorLocation();
}

void ASTUDestructibleObject::ApplyDestructionField(FVector FieldPosition)
{
    if (!FieldSystemComponent || !GeometryCollectionRef || bDestructionApplied) return;

    bDestructionApplied = true;
    GeometryCollectionRef->SetSimulatePhysics(true);

    for (float& Threshold : GeometryCollectionRef->DamageThreshold)
    {
        Threshold = 0.0f;
    }

    FieldSystemComponent->ApplyStrainField(true, FieldPosition, StrainRadius, StrainMagnitude, StrainIterations);
    ApplyImpactImpulse(FieldPosition, BreakImpactImpulseStrength);
    OnDestructionTriggered(FieldPosition);
}

void ASTUDestructibleObject::ApplyPostBreakStrain(FVector FieldPosition, float MagnitudeScale)
{
    if (!FieldSystemComponent || !GeometryCollectionRef) return;

    GeometryCollectionRef->SetSimulatePhysics(true);
    const float FinalMagnitude = FMath::Max(1.0f, StrainMagnitude * FMath::Max(0.01f, MagnitudeScale));
    FieldSystemComponent->ApplyStrainField(true, FieldPosition, StrainRadius, FinalMagnitude, StrainIterations);
    ApplyImpactImpulse(FieldPosition, PostBreakImpactImpulseStrength);
}

void ASTUDestructibleObject::MulticastPostBreakStrain_Implementation(FVector_NetQuantize FieldPosition, float MagnitudeScale)
{
    if (!bIsDestroyed) return;
    ApplyPostBreakStrain(FVector(FieldPosition), MagnitudeScale);
}

void ASTUDestructibleObject::QueuePostBreakStrain(FVector ImpactPosition, float MagnitudeScale)
{
    if (!HasAuthority()) return;

    PendingPostBreakImpactSum += ImpactPosition;
    PendingPostBreakHitCount++;
    PendingPostBreakMagnitudeScale = FMath::Max(PendingPostBreakMagnitudeScale, MagnitudeScale);

    if (!GetWorld() || GetWorld()->GetTimerManager().IsTimerActive(PostBreakBatchTimerHandle)) return;
    GetWorld()->GetTimerManager().SetTimer(
        PostBreakBatchTimerHandle, this, &ASTUDestructibleObject::FlushQueuedPostBreakStrain, PostBreakBatchWindow, false);
}

void ASTUDestructibleObject::FlushQueuedPostBreakStrain()
{
    if (!HasAuthority() || PendingPostBreakHitCount <= 0) return;

    const FVector BatchedImpactPoint = PendingPostBreakImpactSum / FMath::Max(1, PendingPostBreakHitCount);
    const float BatchedMagnitudeScale = FMath::Clamp(PendingPostBreakMagnitudeScale, 0.01f, 1.0f);

    PendingPostBreakImpactSum = FVector::ZeroVector;
    PendingPostBreakHitCount = 0;
    PendingPostBreakMagnitudeScale = 0.0f;

    MulticastPostBreakStrain(FVector_NetQuantize(BatchedImpactPoint), BatchedMagnitudeScale);
}

void ASTUDestructibleObject::ApplyImpactImpulse(FVector ImpactPoint, float ImpulseStrength)
{
    if (!GeometryCollectionRef || !bUseImpactImpulse || ImpulseStrength <= 0.0f) return;

    FVector ImpulseDirection = (ImpactPoint - GetActorLocation()).GetSafeNormal();
    if (ImpulseDirection.IsNearlyZero())
    {
        ImpulseDirection = GetActorForwardVector();
    }

    GeometryCollectionRef->AddImpulseAtLocation(ImpulseDirection * ImpulseStrength, ImpactPoint, NAME_None);
}

void ASTUDestructibleObject::OnRep_Health()
{
    // Override in Blueprint for damage VFX, UI updates
}

void ASTUDestructibleObject::OnRep_IsDestroyed()
{
    if (bIsDestroyed && !bDestructionApplied)
    {
        ApplyDestructionField(FVector(DestructionFieldPosition));
    }
}
