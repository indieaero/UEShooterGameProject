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

	//Every time when game is start 
	Health = MaxHealth;
}

