// Shoot Them Up Game, All Rights Reserved.

#include "Weapon/STUBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"

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
    UE_LOG(LogBaseWeapon, Display, TEXT("Fire!"));

    MakeShot();
}

void ASTUBaseWeapon::MakeShot()
{
    if (!GetWorld()) return;

    const auto Player = Cast<ACharacter>(GetOwner());
    if (!Player) return;

    const auto Controller = Player->GetController<APlayerController>();
    if (!Controller) return;

    FVector ViewLocation;
    FRotator ViewRotation;

    //TODO: see the insides of this function
    Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);

    // Get the position and orientation of the socket (attachment point on the weapon model) for the muzzle (MuzzleSocketName)
    const FTransform SocketTransform = WeaponMesh->GetSocketTransform(MuzzleSocketName);

    // Set the starting point of the trace (TraceStart) to the socket's location (the muzzle)
    const FVector TraceStart = ViewLocation; // SocketTransform.GetLocation();

    // Calculate the shooting direction based on the socket's orientation
    const FVector ShootDirection = ViewRotation.Vector();  // SocketTransform.GetRotation().GetForwardVector();

    // Calculate the end point of the trace
    const FVector TraceEnd = TraceStart + ShootDirection * TraceMaxDistance;

    //This function takes an actor pointer and adds it to the array of actors to ignore during the trace, allowing to ignore our character
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(GetOwner());

    //Perform a line trace to detect a hit along the shot path, using the LineTraceSingleByChannel
    FHitResult HitResult;
    GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);

    if (HitResult.bBlockingHit)
    {
        // Draw debug line from the TraceStart to the TraceEnd for visualization in the world.
        DrawDebugLine(GetWorld(), SocketTransform.GetLocation(), HitResult.ImpactPoint, FColor::Red, false, 3.0f, 0, 3.0f);

        DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 10.0f, 24, FColor::Red, false, 5.0f);

        UE_LOG(LogBaseWeapon, Display, TEXT("Bone: %s"), *HitResult.BoneName.ToString());
    }else
    {
        DrawDebugLine(GetWorld(), SocketTransform.GetLocation(), TraceEnd, FColor::Red, false, 3.0f, 0, 3.0f);
    }
}