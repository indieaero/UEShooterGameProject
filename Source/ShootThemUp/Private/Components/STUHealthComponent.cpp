// Shoot Them Up Game, All Rights Reserved.

#include "Components/STUHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include  "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(logHealthComponent, All, All)

//Sets default values for this component's properties
USTUHealthComponent::USTUHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USTUHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // Every time when game is start
    SetHealth(MaxHealth);

    // Each component has a get owner function that returns a pointer to the owner of this component
    AActor* ComponentOwner = GetOwner();
    if (ComponentOwner)
    {
        ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &USTUHealthComponent::OnTakeAnyDamage);
    }

}

 void USTUHealthComponent::OnTakeAnyDamage(
    AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
     if (Damage <= 0.0f || IsDead() || !GetWorld()) return;

    SetHealth(Health - Damage);
    GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);

    if (IsDead())
    {
        OnDeath.Broadcast();
    }
    //After receiving damage, a timer is set and function HealUpdate starts to be called cyclically
    else if (AutoHeal)
    {
        GetWorld()->GetTimerManager().SetTimer(HealTimerHandle, this, &USTUHealthComponent::HealUpdate, HealUpdateTime, true, HealDelay);
    }
}

void USTUHealthComponent::HealUpdate()
{
    SetHealth(Health + HealModifier);
    // After health change, notify all clients via delegate that health has changed
    OnHealthChanged.Broadcast(Health);

    if (FMath::IsNearlyEqual(Health, MaxHealth) && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);
    }
}

void USTUHealthComponent::SetHealth(float NewHealth)
{
    Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    // After health change, notify all clients via delegate that health has changed
    OnHealthChanged.Broadcast(Health);
}
