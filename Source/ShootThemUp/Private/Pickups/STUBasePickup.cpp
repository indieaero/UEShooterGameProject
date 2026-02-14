// Shoot Them Up Game, All Rights Reserved.

#include "Pickups/STUBasePickup.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogBasePickup, All, All)

ASTUBasePickup::ASTUBasePickup()
{
    PrimaryActorTick.bCanEverTick = true;
    SetReplicates(true);

    CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
    CollisionComponent->InitSphereRadius(50.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
    SetRootComponent(CollisionComponent);
}

void ASTUBasePickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTUBasePickup, bPickupTaken);
}

void ASTUBasePickup::OnRep_PickupTaken()
{
    // Update visibility/collision on clients when pickup state replicates
    if (bPickupTaken)
    {
        if (CollisionComponent)
        {
            CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
        }
        if (GetRootComponent())
        {
            GetRootComponent()->SetVisibility(false, true);
        }
    }
    else
    {
        if (CollisionComponent)
        {
            CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
        }
        if (GetRootComponent())
        {
            GetRootComponent()->SetVisibility(true, true);
        }
    }
}

void ASTUBasePickup::BeginPlay()
{
    Super::BeginPlay();

    check(CollisionComponent);

    GenerateRotationYaw();
}

void ASTUBasePickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);
    if (!HasAuthority()) return;  // Only server gives pickup; bPickupTaken replicates

    const auto Pawn = Cast<APawn>(OtherActor);
    if (Pawn && GivePickupTo(Pawn))
    {
        PickupWasTaken();
    }
}

void ASTUBasePickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AddActorLocalRotation(FRotator(0.0f, RotationYaw, 0.0f));
}

bool ASTUBasePickup::CouldBeTaken() const
{
    return !GetWorldTimerManager().IsTimerActive(RespawnTimeHandle);
}

bool ASTUBasePickup::GivePickupTo(APawn* PlayerPawn)
{
    return false;
}

void ASTUBasePickup::PickupWasTaken()
{
    bPickupTaken = true;
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    }
    if (GetRootComponent())
    {
        GetRootComponent()->SetVisibility(false, true);
    }
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(RespawnTimeHandle, this, &ASTUBasePickup::Respawn, RespawnTime);
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupTakenSound, GetActorLocation());
    }
}

void ASTUBasePickup::Respawn()
{
    bPickupTaken = false;
    GenerateRotationYaw();
    if (CollisionComponent)
    {
        CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
    }
    if (GetRootComponent())
    {
        GetRootComponent()->SetVisibility(true, true);
    }
}

void ASTUBasePickup::GenerateRotationYaw()  
{
    const auto Direction = FMath::RandBool() ? 1.0f : -1.0f;
    RotationYaw = FMath::RandRange(1.0f, 2.0f) * Direction;
}
