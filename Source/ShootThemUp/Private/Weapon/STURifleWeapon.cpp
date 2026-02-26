// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STURifleWeapon.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Weapon/Components/STUWeaponFXComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/STUPlayerController.h"

ASTURifleWeapon::ASTURifleWeapon()
{
    WeaponFXComponent = CreateDefaultSubobject<USTUWeaponFXComponent>("WeaponFXComponent");
}

void ASTURifleWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTURifleWeapon, bIsFiring);
}

void ASTURifleWeapon::StartFire()
{
    Super::StartFire();
    if (!HasAuthority()) return;
    bIsFiring = true;
    InitFX();
    GetWorldTimerManager().SetTimer(ShotTimerHandle, this, &ASTURifleWeapon::MakeShot, TimeBetweenShots, true);
    MakeShot();
}

void ASTURifleWeapon::StopFire()
{
    if (HasAuthority())
    {
        bIsFiring = false;
    }
    GetWorldTimerManager().ClearTimer(ShotTimerHandle);
    SetFXActive(false);
}

void ASTURifleWeapon::OnRep_IsFiring()
{
    if (bIsFiring)
    {
        InitFX();
    }
    SetFXActive(bIsFiring);
}

void ASTURifleWeapon::Zoom(bool IsEnabled) 
{
    const auto Controller = GetController();
    if (!Controller) return;

    const auto PlayerController = Cast<APlayerController>(Controller);
    if (!PlayerController) return;

    if(IsEnabled)
    {
        DefaultCameraFOV = PlayerController->PlayerCameraManager->GetFOVAngle();
    }

    PlayerController->PlayerCameraManager->SetFOV(IsEnabled ? FOVZoomAngle : DefaultCameraFOV);
}

void ASTURifleWeapon::BeginPlay()
{
    Super::BeginPlay();

    check(WeaponFXComponent);
}

void ASTURifleWeapon::MakeShot()
{
    if (!HasAuthority()) return;  // Server: trace, damage, ammo; then MulticastPlayShotFX for clients
    if (!GetWorld() || IsAmmoEmpty())
    {
        StopFire();
        return;
    }

    FVector TraceStart, TraceEnd;
    if (!GetTraceData(TraceStart, TraceEnd))
    {
        StopFire();
        return;
    }

    FHitResult HitResult;
    MakeHit(HitResult, TraceStart, TraceEnd);

    FVector TraceFXEnd = TraceEnd;
    FVector ImpactPoint = TraceEnd;
    bool bBlockingHit = HitResult.bBlockingHit;

    if (HitResult.bBlockingHit)
    {
        TraceFXEnd = HitResult.ImpactPoint;
        ImpactPoint = HitResult.ImpactPoint;
        MakeDamage(HitResult);
        if (WeaponFXComponent)
        {
            WeaponFXComponent->PlayImpactFX(HitResult);
        }
        OnShotHit.Broadcast(HitResult);
    }
    MulticastPlayShotFX(GetMuzzleWorldLocation(), TraceFXEnd, ImpactPoint, bBlockingHit);
    DecreaseAmmo();
}

void ASTURifleWeapon::MulticastPlayShotFX_Implementation(const FVector& TraceStart, const FVector& TraceEnd, const FVector_NetQuantize& ImpactPoint, bool bBlockingHit)
{
    // Local player: use local muzzle position so trace FX comes from barrel, not server position
    FVector FXTraceStart = TraceStart;
    const APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn && OwnerPawn->IsLocallyControlled())
    {
        //recoil only for the shooting player
        if (ASTUPlayerController* PC = Cast<ASTUPlayerController>(OwnerPawn->GetController()))
        {
            const float PitchRecoil = RecoilPitchPerShot;
            const float YawRecoil = FMath::RandRange(-RecoilYawRandom, RecoilYawRandom);
            PC->ApplyRecoil(PitchRecoil, YawRecoil);
        }

        FXTraceStart = GetMuzzleWorldLocation();
        PlayRifleCameraShake();
    }

    SpawnTraceFX(FXTraceStart, TraceEnd);
    if (bBlockingHit && WeaponFXComponent)
    {
        FHitResult HitResult;
        HitResult.bBlockingHit = true;
        HitResult.ImpactPoint = FVector(ImpactPoint);
        HitResult.ImpactNormal = (FXTraceStart - ImpactPoint).GetSafeNormal();
        WeaponFXComponent->PlayImpactFX(HitResult);
    }
}

bool ASTURifleWeapon::GetTraceData(FVector& TraceStart, FVector& TraceEnd) const
{
    FVector ViewLocation;
    FRotator ViewRotation;

    if (!GetPlayerViewPoint(ViewLocation, ViewRotation)) return false;

    // Get the position and orientation of the socket (attachment point on the weapon model) for the muzzle (MuzzleSocketName)
    // const FTransform SocketTransform = WeaponMesh->GetSocketTransform(MuzzleSocketName);

    // Set the starting point of the trace (TraceStart) to the socket's location (the muzzle)
    TraceStart = ViewLocation;  // SocketTransform.GetLocation();

    // Calculating bullet spread
    const auto HalfRad = FMath::DegreesToRadians(BulletSpread);

    // Calculate the shooting direction based on the socket's orientation
    const FVector ShootDirection = FMath::VRandCone(ViewRotation.Vector(), HalfRad);  // SocketTransform.GetRotation().GetForwardVector();

    // Calculate the end point of the trace
    TraceEnd = TraceStart + ShootDirection * TraceMaxDistance;
    return true;
}

void ASTURifleWeapon::MakeDamage(const FHitResult& HitResult)
{
    const auto DamageActor = HitResult.GetActor();
    if (!DamageActor) return;

    FPointDamageEvent PointDamageEvent;
    PointDamageEvent.HitInfo = HitResult;
    DamageActor->TakeDamage(DamageAmount, PointDamageEvent, GetController(), this);
}

void ASTURifleWeapon::InitFX() 
{
    if (!MuzzleFXComponent)
    {
        MuzzleFXComponent = SpawnMuzzleFX();
    }

    if(!FireAudioComponent)
    {
        FireAudioComponent = UGameplayStatics::SpawnSoundAttached(FireSound, WeaponMesh, MuzzleSocketName);
    }
    SetFXActive(true);
}

void ASTURifleWeapon::SetFXActive(bool IsActive)
{
    if (MuzzleFXComponent)
    {
        MuzzleFXComponent->SetPaused(!IsActive);
        MuzzleFXComponent->SetVisibility(IsActive, true);
    }

    if(FireAudioComponent)
    {
        FireAudioComponent->SetPaused(!IsActive);
    }
}

void ASTURifleWeapon::SpawnTraceFX(const FVector& TraceStart, const FVector& TraceEnd) 
{
    const auto TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TraceFX, TraceStart);
    if (TraceFXComponent)
    {
        TraceFXComponent->SetNiagaraVariableVec3(TraceTargetName, TraceEnd); 
    }
}

void ASTURifleWeapon::PlayRifleCameraShake() 
{
    const auto Player = Cast<APawn>(GetOwner());
    if (!Player) return;

    const auto Controller = Player->GetController<APlayerController>();
    if (!Controller || !Controller->PlayerCameraManager) return;

    Controller->PlayerCameraManager->StartCameraShake(CameraShake);
}

AController* ASTURifleWeapon::GetController() const
{
    const auto Pawn = Cast<APawn>(GetOwner());
    return Pawn ? Pawn->GetController() : nullptr;
}