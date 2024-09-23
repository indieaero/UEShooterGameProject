// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STUBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "STUHealthComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"


class USTUHealthComponent;
class ASTUBaseCharacter;

DEFINE_LOG_CATEGORY_STATIC(LogBaseWeapon, All, All);

ASTUBaseWeapon::ASTUBaseWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
    SetRootComponent(WeaponMesh);
}

void ASTUBaseWeapon::BeginPlay()
{
    Super::BeginPlay();
    check(WeaponMesh);
}

void ASTUBaseWeapon::Fire()
{
    MakeShot();
}

void ASTUBaseWeapon::MakeShot()
{
    if (!GetWorld()) return;

    FVector TraceStart, TraceEnd;
    if (!GetTraceData(TraceStart, TraceEnd)) return;

    FHitResult HitResult;

    MakeHit(HitResult, TraceStart, TraceEnd);

    AActor* HitActor = HitResult.GetActor();

    if (HitActor)
    {
        USTUHealthComponent* HealthComponent = HitActor->FindComponentByClass<USTUHealthComponent>();
        if (HealthComponent)
        {
            AController* InstigatingController = GetPlayerController();

            FPointDamageEvent DamageEvent;
            HitActor->TakeDamage(220.0f, DamageEvent, InstigatingController, this);
        }
    }

    if (HitResult.bBlockingHit)
    {
        // Draw debug line from the TraceStart to the TraceEnd for visualization in the world.
        DrawDebugLine(GetWorld(), GetMuzzleWorldLocation(), HitResult.ImpactPoint, FColor::Red, false, 3.0f, 0, 3.0f);

        DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 24, FColor::Red, false, 5.0f);
    }
    else
    {
        DrawDebugLine(GetWorld(), GetMuzzleWorldLocation(), TraceEnd, FColor::Red, false, 3.0f, 0, 3.0f);
    }
}

APlayerController* ASTUBaseWeapon::GetPlayerController() const
{
    const auto Player = Cast<ACharacter>(GetOwner());
    if (!Player) return nullptr;

    return Player->GetController<APlayerController>();
}

bool ASTUBaseWeapon::GetPlayerViewPoint(FVector& ViewLocation, FRotator& ViewRotation) const
{
    const auto Controller = GetPlayerController();
    if (!Controller) return false;

    // TODO: see the insides of this function
    Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
    return true;
}

FVector ASTUBaseWeapon::GetMuzzleWorldLocation() const
{
    return WeaponMesh->GetSocketLocation(MuzzleSocketName);
}

bool ASTUBaseWeapon::GetTraceData(FVector& TraceStart, FVector& TraceEnd) const
{
    FVector ViewLocation;
    FRotator ViewRotation;

    if(!GetPlayerViewPoint(ViewLocation, ViewRotation)) return false;

    // Get the position and orientation of the socket (attachment point on the weapon model) for the muzzle (MuzzleSocketName)
    //const FTransform SocketTransform = WeaponMesh->GetSocketTransform(MuzzleSocketName);

    // Set the starting point of the trace (TraceStart) to the socket's location (the muzzle)
    TraceStart = ViewLocation; // SocketTransform.GetLocation();

    // Calculate the shooting direction based on the socket's orientation
    const FVector ShootDirection = ViewRotation.Vector();  // SocketTransform.GetRotation().GetForwardVector();

    // Calculate the end point of the trace
    TraceEnd = TraceStart + ShootDirection * TraceMaxDistance;
    return true;
}

void ASTUBaseWeapon::MakeHit(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd)
{
    if (!GetWorld()) return;

    // This function takes an actor pointer and adds it to the array of actors to ignore during the trace, allowing to ignore our character
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(GetOwner());

    // Perform a line trace to detect a hit along the shot path, using the LineTraceSingleByChannel
    GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);
}