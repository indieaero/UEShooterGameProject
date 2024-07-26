// Shoot Them Up Game, All Rights Reserved.

#include "Components/STUHealthComponent.h"

//Sets default values for this component's properties
USTUHealthComponent::USTUHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USTUHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // Every time when game is start
    Health = MaxHealth;

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
     Health -= Damage;
}