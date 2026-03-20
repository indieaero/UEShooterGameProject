// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STULauncherWeapon.h"
#include "STUProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Player/STUPlayerController.h"

void ASTULauncherWeapon::StartFire()
{
    MakeShot();
}

void ASTULauncherWeapon::MakeShot()
{
    if (!HasAuthority()) return;  // Server: spawn projectile and ammo; MulticastPlayFireFX for clients
    if (!GetWorld()) return;

    if (IsAmmoEmpty())
    {
        UGameplayStatics::SpawnSoundAtLocation(GetWorld(), NoAmmoSound, GetActorLocation());
        return;
    }

    FVector TraceStart, TraceEnd;
    if (!GetTraceData(TraceStart, TraceEnd)) return;

    FHitResult HitResult;
    MakeHit(HitResult, TraceStart, TraceEnd);

    const FVector EndPoint = HitResult.bBlockingHit ? HitResult.ImpactPoint : TraceEnd;
    const FVector Direction = (EndPoint - GetMuzzleWorldLocation()).GetSafeNormal();

    const FTransform SpawnTransform(FRotator::ZeroRotator, GetMuzzleWorldLocation());

    ASTUProjectile* Projectile = GetWorld()->SpawnActorDeferred<ASTUProjectile>(ProjectileClass, SpawnTransform);
    if (Projectile)
    {
        Projectile->SetShotDirection(Direction);
        Projectile->SetOwner(GetOwner());
        Projectile->FinishSpawning(SpawnTransform);
    }

    DecreaseAmmo();
    SpawnMuzzleFX();
    UGameplayStatics::SpawnSoundAttached(FireSound, WeaponMesh, MuzzleSocketName);
    MulticastPlayFireFX();
}

void ASTULauncherWeapon::PlayLauncherCameraShake() 
{
    const auto Player = Cast<APawn>(GetOwner());
    if (!Player) return;

    const auto Controller = Player->GetController<APlayerController>();
    if (!Controller || !Controller->PlayerCameraManager) return;

    Controller->PlayerCameraManager->StartCameraShake(CameraShake);
}

void ASTULauncherWeapon::MulticastPlayFireFX_Implementation()
{
    const APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn && OwnerPawn->IsLocallyControlled())
    {
        if (ASTUPlayerController* PC = Cast<ASTUPlayerController>(OwnerPawn->GetController()))
        {
            const float PitchRecoil = RecoilPitchPerShot;
            const float YawRecoil = FMath::RandRange(-RecoilYawRandom, RecoilYawRandom);
            PC->ApplyRecoil(PitchRecoil, YawRecoil);
        }
    }

    // Clients play muzzle FX and sound (server already did in MakeShot)
    if (!HasAuthority())
    {
        SpawnMuzzleFX();
        if (FireSound && WeaponMesh)
        {
            UGameplayStatics::SpawnSoundAttached(FireSound, WeaponMesh, MuzzleSocketName);
        }
    }

    PlayLauncherCameraShake();
}
