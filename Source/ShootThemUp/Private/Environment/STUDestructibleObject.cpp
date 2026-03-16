// Shoot Them Up Game, All Rights Reserved.

#include "Environment/STUDestructibleObject.h"
#include "Field/FieldSystemComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

namespace
{
    constexpr float InvulnerableDamageThreshold = 1e9f;
}

ASTUDestructibleObject::ASTUDestructibleObject()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    FieldSystemComponent = CreateDefaultSubobject<UFieldSystemComponent>("FieldSystemComponent");
    FieldSystemComponent->SetupAttachment(RootComponent);
}

void ASTUDestructibleObject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTUDestructibleObject, Health);
    DOREPLIFETIME(ASTUDestructibleObject, bIsDestroyed);
}

void ASTUDestructibleObject::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        Health = MaxHealth;
    }

    if (!GeometryCollectionRef)
    {
        GeometryCollectionRef = FindComponentByClass<UGeometryCollectionComponent>();
    }

    if (GeometryCollectionRef)
    {
        for (float& Threshold : GeometryCollectionRef->DamageThreshold)
        {
            Threshold = InvulnerableDamageThreshold;
        }
    }
}

float ASTUDestructibleObject::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || bIsDestroyed) return 0.0f;

    // Save last hit point for field position (when bUseLastHitPoint is true)
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        LastDamageImpactPoint = static_cast<const FPointDamageEvent&>(DamageEvent).HitInfo.ImpactPoint;
    }

    // Don't call Super - we handle damage ourselves to prevent propagation to Geometry Collection
    Health = FMath::Max(0.0f, Health - DamageAmount);

    CheckAndTriggerDestruction();
    return DamageAmount;
}

void ASTUDestructibleObject::TriggerDestructionAt(FVector Location)
{
    if (!HasAuthority() || bIsDestroyed) return;

    bIsDestroyed = true;
    Health = 0.0f;
    MulticastApplyDestruction(FVector_NetQuantize(Location));
}

void ASTUDestructibleObject::CheckAndTriggerDestruction()
{
    if (!HasAuthority() || bIsDestroyed || Health > 0.0f) return;

    bIsDestroyed = true;
    MulticastApplyDestruction(FVector_NetQuantize(GetDestructionFieldPosition()));
}

FVector ASTUDestructibleObject::GetDestructionFieldPosition() const
{
    if (bUseLastHitPoint && LastDamageImpactPoint != FVector::ZeroVector)
    {
        return LastDamageImpactPoint;
    }
    return GetActorLocation();
}

void ASTUDestructibleObject::MulticastApplyDestruction_Implementation(FVector_NetQuantize FieldPosition)
{
    ApplyDestructionField(FVector(FieldPosition));
}

void ASTUDestructibleObject::ApplyDestructionField(FVector FieldPosition)
{
    if (!FieldSystemComponent || !GeometryCollectionRef || bDestructionApplied) return;

    bDestructionApplied = true;

    for (float& Threshold : GeometryCollectionRef->DamageThreshold)
    {
        Threshold = 0.0f;
    }

    FieldSystemComponent->ApplyStrainField(true, FieldPosition, StrainRadius, StrainMagnitude, StrainIterations);
    OnDestructionTriggered(FieldPosition);
}

void ASTUDestructibleObject::OnRep_Health()
{
    // Override in Blueprint for damage VFX, UI updates
}

void ASTUDestructibleObject::OnRep_IsDestroyed()
{
    if (bIsDestroyed && !bDestructionApplied)
    {
        ApplyDestructionField(GetDestructionFieldPosition());
    }
}
