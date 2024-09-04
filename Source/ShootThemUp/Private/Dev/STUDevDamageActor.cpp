// Shoot Them Up Game, All Rights Reserved.


#include "Dev/STUDevDamageActor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASTUDevDamageActor::ASTUDevDamageActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create a default subobject for the scene component and set it as the root component
    SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
    SetRootComponent(SceneComponent);
}

// Called when the game starts or when spawned
void ASTUDevDamageActor::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void ASTUDevDamageActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 24, SphereColor);

    //Call function from GameplayStatics that apply damage to all actors every tick
    UGameplayStatics::ApplyRadialDamage(GetWorld(), Damage, GetActorLocation(), Radius, 
        DamageType, {}, this, nullptr, DoFullDamage);
}

