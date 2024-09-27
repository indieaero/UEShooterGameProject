// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STULauncherWeapon.h"
#include "STUProjectile.h"
#include "Kismet/GameplayStatics.h"

void ASTULauncherWeapon::StartFire()
{
    MakeShot();
}

void ASTULauncherWeapon::MakeShot()
{
    //variable responsible for the initial transformation of the projectile
    const FTransform SpawnTransform(FRotator::ZeroRotator, GetMuzzleWorldLocation());

    //Rocket Actor pointer
    auto Projectile = UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ProjectileClass, SpawnTransform);
    //set projectile params

    UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);
}
