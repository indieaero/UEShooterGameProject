// Shoot Them Up Game, All Rights Reserved.

#include "Components/STUHealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Camera/CameraShakeBase.h"
#include "STUGameModeBase.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/DamageEvents.h"
#include "Perception/AISense_Damage.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(logHealthComponent, All, All)

// Sets default values for this component's properties
USTUHealthComponent::USTUHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);
}

bool USTUHealthComponent::TryToAddHealth(float HealthAmount)
{
    // Only server can change health
    if (GetOwner() && !GetOwner()->HasAuthority()) return false;

    if (Health == MaxHealth)
    {
        return false;
    }
    Health += HealthAmount;
    return true;
}

void USTUHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USTUHealthComponent, Health);
    DOREPLIFETIME(USTUHealthComponent, MaxHealth);
}

void USTUHealthComponent::OnRep_Health() 
{
    const float PreviousHealth = LastReplicatedHealth;
    LastReplicatedHealth = Health;
    const float HealthDelta = (PreviousHealth >= 0.0f) ? (Health - PreviousHealth) : 0.0f;

    OnHealthChanged.Broadcast(Health, HealthDelta);

    if (!IsDead() && HealthDelta < 0.0f)
    {
        PlayCameraShake();
    }

    if (IsDead())
    {
        OnDeath.Broadcast();
    }
}

void USTUHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    check(MaxHealth > 0);

    // Only server can set initial health value
    AActor* ComponentOwner = GetOwner();
    if (ComponentOwner && ComponentOwner->HasAuthority())
    {
        SetHealth(MaxHealth);
    }

    // Each component has a get owner function that returns a pointer to the owner of this component
    if (ComponentOwner)
    {
        ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &USTUHealthComponent::OnTakeAnyDamage);
        ComponentOwner->OnTakePointDamage.AddDynamic(this, &USTUHealthComponent::OnTakePointDamage);
        ComponentOwner->OnTakeRadialDamage.AddDynamic(this, &USTUHealthComponent::OnTakeRadialDamage);
    }
}

void USTUHealthComponent::HealUpdate()
{
    SetHealth(Health + HealModifier);

    if (FMath::IsNearlyEqual(Health, MaxHealth) && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);
    }
}

void USTUHealthComponent::SetHealth(float NewHealth)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;  // Server authoritative; Health replicates
    const float NextHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    const float HealthDelta = NextHealth - Health;
    Health = NextHealth;
    OnHealthChanged.Broadcast(Health, HealthDelta);
}

void USTUHealthComponent::PlayCameraShake()
{
    if (IsDead()) return;

    const auto Player = Cast<APawn>(GetOwner());
    if (!Player) return;

    const auto Controller = Player->GetController<APlayerController>();
    if (!Controller || !Controller->PlayerCameraManager) return;

    Controller->PlayerCameraManager->StartCameraShake(CameraShake);
}

void USTUHealthComponent::Killed(AController* KillerController)
{
    if (!GetWorld()) return;

    const auto GameMode = Cast<ASTUGameModeBase>(GetWorld()->GetAuthGameMode());
    if (!GameMode) return;

    const auto Player = Cast<APawn>(GetOwner());
    const auto VictimController = Player ? Player->GetController() : nullptr;

    GameMode->Killed(KillerController, VictimController);
}

void USTUHealthComponent::OnTakeAnyDamage(
    AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    UE_LOG(logHealthComponent, Display, TEXT("Point any damage: %f"), Damage);
}

void USTUHealthComponent::OnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation,
    class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType,
    AActor* DamageCauser)
{
    const auto FinalDamage = Damage * GetPointDamageModifier(DamagedActor, BoneName);
    UE_LOG(logHealthComponent, Display, TEXT("Point damage: %f, final damage: %f, bone: %s"), Damage, FinalDamage, *BoneName.ToString());
    ApplyDamage(FinalDamage, InstigatedBy);
}

void USTUHealthComponent::OnTakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin,
    const FHitResult&, class AController* InstigatedBy, AActor* DamageCauser)
{
    UE_LOG(logHealthComponent, Display, TEXT("Point radial damage: %f"), Damage);
    ApplyDamage(Damage, InstigatedBy);
}

void USTUHealthComponent::ApplyDamage(float Damage, AController* InstigatedBy)
{
    if (Damage <= 0.0f || IsDead() || !GetWorld()) return;

    SetHealth(Health - Damage);
    GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);

    if (IsDead())
    {
        Killed(InstigatedBy);
        OnDeath.Broadcast();
    }
    // After receiving damage, a timer is set and function HealUpdate starts to be called cyclically
    else if (AutoHeal)
    {
        GetWorld()->GetTimerManager().SetTimer(HealTimerHandle, this, &USTUHealthComponent::HealUpdate, HealUpdateTime, true, HealDelay);
    }

    PlayCameraShake();
    ReportDamageEvent(Damage, InstigatedBy);
}

float USTUHealthComponent::GetPointDamageModifier(AActor* DamagedActor, const FName& BoneName) const
{
    const auto Character = Cast<ACharacter>(DamagedActor);
    if (!Character ||             //
        !Character->GetMesh() ||  //
        !Character->GetMesh()->GetBodyInstance(BoneName))
        return 1.0f;

    const auto PhysMaterial = Character->GetMesh()->GetBodyInstance(BoneName)->GetSimplePhysicalMaterial();
    if (!PhysMaterial || !DamageModifiers.Contains(PhysMaterial)) return 1.0f;

    return DamageModifiers[PhysMaterial];
}

void USTUHealthComponent::ReportDamageEvent(float Damage, AController* InstigatedBy)
{
    if (!InstigatedBy || !InstigatedBy->GetPawn() || !GetOwner()) return;

    UAISense_Damage::ReportDamageEvent(GetWorld(),    //
        GetOwner(),                                   //
        InstigatedBy->GetPawn(),                      //
        Damage,                                       //
        InstigatedBy->GetPawn()->GetActorLocation(),  //
        GetOwner()->GetActorLocation());
}