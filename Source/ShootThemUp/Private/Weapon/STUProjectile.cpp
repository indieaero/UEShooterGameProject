// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STUProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Components/STUWeaponFXComponent.h"
#include "Net/UnrealNetwork.h"

ASTUProjectile::ASTUProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    SetReplicates(true);         // Spawned on server; visible on all clients
    SetReplicateMovement(true);  // Smooth position sync

    CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
    // set radius for collision
    CollisionComponent->InitSphereRadius(5.0f);
    SetRootComponent(CollisionComponent);

    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
    CollisionComponent->bReturnMaterialOnMove = true;
    SetRootComponent(CollisionComponent);

    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
    MovementComponent->InitialSpeed = 2000.f;
    MovementComponent->ProjectileGravityScale = 0.0f;

    WeaponFXComponent = CreateDefaultSubobject<USTUWeaponFXComponent>("WeaponFXComponent");

    TraceFX = CreateDefaultSubobject<UNiagaraComponent>("TraceFX");
    TraceFX->SetupAttachment(RootComponent);
}

void ASTUProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTUProjectile, ShotDirection);  // Client needs direction to set velocity in BeginPlay
}

void ASTUProjectile::BeginPlay()
{
    Super::BeginPlay();

    check(MovementComponent);
    check(CollisionComponent);
    check(WeaponFXComponent);

    TraceFX->Activate(true);

    MovementComponent->Velocity = ShotDirection * MovementComponent->InitialSpeed;
    if (GetOwner())
    {
        CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
    }
    CollisionComponent->OnComponentHit.AddDynamic(this, &ASTUProjectile::OnProjectileHit);

    SetLifeSpan(LifeSeconds);
}

void ASTUProjectile::OnProjectileHit(
    UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!GetWorld()) return;

    MovementComponent->StopMovementImmediately();

    if (HasAuthority())  // Server applies damage; clients only play impact FX
    {
        UGameplayStatics::ApplyRadialDamage(GetWorld(),
            DamageAmount,
            GetActorLocation(),
            DamageRadius,
            UDamageType::StaticClass(),
            {},
            this,
            GetController(),
            DoFullDamage);
    }

    if (WeaponFXComponent)
    {
        WeaponFXComponent->PlayImpactFX(Hit);
    }
    if (TraceFX)
    {
        TraceFX->Deactivate();
    }

    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetLifeSpan(3.0f);
}

AController* ASTUProjectile::GetController() const
{
    const auto Pawn = Cast<APawn>(GetOwner());
    return Pawn ? Pawn->GetController() : nullptr;
}