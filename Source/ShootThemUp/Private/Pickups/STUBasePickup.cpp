// Shoot Them Up Game, All Rights Reserved.

#include "Pickups/STUBasePickup.h"
#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "Player/STUBaseCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogBasePickup, All, All)

ASTUBasePickup::ASTUBasePickup()
{
    PrimaryActorTick.bCanEverTick = true;
    SetReplicates(true);

    CollisionComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
    CollisionComponent->InitSphereRadius(50.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
    // Projectiles use ECC_GameTraceChannel1; ignore so they do not detonate on pickups.
    CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
    CollisionComponent->SetGenerateOverlapEvents(true);
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
    if (bPickupTaken && !HasAuthority())
    {
        if (CollisionComponent)
        {
            CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
        }
        if (GetRootComponent())
        {
            GetRootComponent()->SetVisibility(false, true);
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), PickupTakenSound, GetActorLocation());
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

    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASTUBasePickup::OnSphereBeginOverlap);

    GenerateRotationYaw();
}

void ASTUBasePickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ASTUBasePickup::OnSphereBeginOverlap);
    }
    Super::EndPlay(EndPlayReason);
}

void ASTUBasePickup::AuthorityTryGiveToPawn(APawn* Pawn)
{
    if (!HasAuthority() || !Pawn || bPickupTaken) return;

    if (GivePickupTo(Pawn))
    {
        PickupWasTaken();
    }
}

void ASTUBasePickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    APawn* OverlapPawn = Cast<APawn>(OtherActor);
    if (!OverlapPawn && OtherComp)
    {
        OverlapPawn = Cast<APawn>(OtherComp->GetOwner());
    }
    if (!OverlapPawn) return;

    if (HasAuthority())
    {
        AuthorityTryGiveToPawn(OverlapPawn);
        return;
    }

    if (OverlapPawn->IsLocallyControlled())
    {
        if (ASTUBaseCharacter* Character = Cast<ASTUBaseCharacter>(OverlapPawn))
        {
            Character->ServerTryPickupActor(this);
        }
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
