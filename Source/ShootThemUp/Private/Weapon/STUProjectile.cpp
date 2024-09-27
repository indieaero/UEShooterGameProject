// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STUProjectile.h"
#include "Components/SphereComponent.h"

ASTUProjectile::ASTUProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
    //set radius for collision
    CollisionComponent->InitSphereRadius(5.0f);
    SetRootComponent(CollisionComponent);
}

void ASTUProjectile::BeginPlay()
{
    Super::BeginPlay();
}
