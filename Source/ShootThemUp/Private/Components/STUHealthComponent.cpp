// Shoot Them Up Game, All Rights Reserved.

#include "Components/STUHealthComponent.h"
#include "GameFramework/Actor.h"
#include "Dev/STUFireDamageType.h"
#include "Dev/STUIceDamageType.h"

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
    Health = MaxHealth;

    // Each component has a get owner function that returns a pointer to the owner of this component
    AActor* ComponentOwner = GetOwner();
    if (ComponentOwner)
    {
        ComponentOwner->OnTakeAnyDamage.AddDynamic(this, &USTUHealthComponent::OnTakeAnyDamage);
    }
}

 void USTUHealthComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    Health -= Damage;
    UE_LOG(logHealthComponent, Display, TEXT("Damage: %f"), Damage);

    if (DamageType)
    {
        if (DamageType->IsA<USTUFireDamageType>())
        {
            UE_LOG(logHealthComponent, Display, TEXT("So hoooooot ! %f"), Damage);
        }
        else if (DamageType->IsA<USTUIceDamageType>())
        {
            UE_LOG(logHealthComponent, Display, TEXT("So coooooold ! %f"), Damage);
        }
    }
}