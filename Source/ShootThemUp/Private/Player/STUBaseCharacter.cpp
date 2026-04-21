// Shoot Them Up Game, All* Rights Reserved.

#include "Player/STUBaseCharacter.h"
#include "Components/STUCharacterMovementComponent.h"
#include "Components/STUHealthComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/STUWeaponComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "Pickups/STUBasePickup.h"

// TODO: read about define log category
DEFINE_LOG_CATEGORY_STATIC(LogBaseCharacter, All, All);

// Sets default values in constructor
ASTUBaseCharacter::ASTUBaseCharacter(const FObjectInitializer& ObjInit)
    : Super(ObjInit.SetDefaultSubobjectClass<USTUCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    HealthComponent = CreateDefaultSubobject<USTUHealthComponent>("HealthComponent");
    WeaponComponent = CreateDefaultSubobject<USTUWeaponComponent>("WeaponComponent");

    // WorldDynamic pickups use overlap; default capsule blocks WorldDynamic so BeginOverlap never fires.
    // Projectiles use ECC_GameTraceChannel1 (Enemy) so they still block against the capsule.
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    }
}

// Called when the game starts or when spawned
void ASTUBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    // macros for check that Health Component and HealthTextComponent is not null (work only in Debug and Development builds)
    check(HealthComponent);
    check(GetCharacterMovement());
    check(GetMesh());

    OnHealthChanged(HealthComponent->GetHealth(), 0.0f);
    HealthComponent->OnDeath.AddUObject(this, &ASTUBaseCharacter::OnDeath);

    // Bind a new function to the OnHealthChanged delegate and update the text component only when the value actually changes, not every
    // frame
    HealthComponent->OnHealthChanged.AddUObject(this, &ASTUBaseCharacter::OnHealthChanged);

    LandedDelegate.AddDynamic(this, &ASTUBaseCharacter::OnGroundLanded);

    // Send initial view rotation to server so remote clients get correct weapon pose from first frame
    if (IsLocallyControlled() && IsPlayerControlled() && GetController())
    {
        ServerUpdateViewRotation(GetControlRotation());
    }

    if (PlayerSpawnSound && GetNetMode() != NM_DedicatedServer)
    {
        // Attach spawn voice to character so it follows movement during playback.
        UGameplayStatics::SpawnSoundAttached(PlayerSpawnSound, GetRootComponent());
    }
}

void ASTUBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTUBaseCharacter, TeamColor);
    DOREPLIFETIME(ASTUBaseCharacter, ReplicatedViewRotation);   // Remote clients: correct weapon aim pose
    DOREPLIFETIME(ASTUBaseCharacter, bReplicatedViewRotationSet);
}

FRotator ASTUBaseCharacter::GetBaseAimRotation() const
{
    if (IsLocallyControlled())
    {
        return GetControlRotation();
    }
    // Before first replication: return horizontal by body yaw so remote pawn doesn't show "weapon up" default
    if (!bReplicatedViewRotationSet)
    {
        return FRotator(0.f, GetActorRotation().Yaw, 0.f);
    }
    return ReplicatedViewRotation;
}

FRotator ASTUBaseCharacter::GetAimRotationRelativeToCharacter() const
{
    // Delta from body forward; (0,0,0) = aiming forward = Aim Offset center pose
    const FRotator AimWorld = GetBaseAimRotation();
    const FRotator BodyWorld = GetActorRotation();
    return (AimWorld - BodyWorld).GetNormalized();
}

// Called only on the client of the player who took damage 
void ASTUBaseCharacter::ClientOnDamageTaken_Implementation() 
{
    if (HealthComponent)
    {
        HealthComponent->NotifyClientDamageTaken();
    }
}

void ASTUBaseCharacter::MulticastPlayPlayerDeathSound_Implementation()
{
    if (PlayerDeathSound && GetNetMode() != NM_DedicatedServer)
    {
        UGameplayStatics::SpawnSoundAttached(PlayerDeathSound, GetRootComponent());
    }
}

void ASTUBaseCharacter::MulticastPlayPlayerHitPainSound_Implementation()
{
    if (PlayerHitPainSound && GetNetMode() != NM_DedicatedServer)
    {
        UGameplayStatics::SpawnSoundAttached(PlayerHitPainSound, GetRootComponent());
    }
}

void ASTUBaseCharacter::ServerUpdateViewRotation_Implementation(FRotator NewRotation)
{
    ReplicatedViewRotation = NewRotation;
    bReplicatedViewRotationSet = true;
}

void ASTUBaseCharacter::OnRep_TeamColor()
{
    if (GetMesh())
    {
        const auto MaterialInst = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
        if (MaterialInst)
        {
            MaterialInst->SetVectorParameterValue(MaterialColorName, TeamColor);
        }
    }
}

void ASTUBaseCharacter::OnHealthChanged(float Health, float HealthDelta)
{
    // Hit pain is played from USTUHealthComponent::ApplyDamage (TryPlayDamageHitPainSound) so healing/pickups never trigger it.
}

void ASTUBaseCharacter::TryPlayDamageHitPainSound()
{
    if (!HasAuthority())
    {
        return;
    }

    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (CurrentTime - LastPlayerHitPainSoundTime >= PlayerHitPainSoundCooldown)
    {
        LastPlayerHitPainSoundTime = CurrentTime;
        MulticastPlayPlayerHitPainSound();
    }
}

void ASTUBaseCharacter::OnGroundLanded(const FHitResult& Hit)
{
    if (!HasAuthority()) return;  // Server applies fall damage; health replicates
    const float FallVelocityZ = -GetVelocity().Z;
    if (FallVelocityZ < LandedDamageVelocity.X) return;

    const float FallDamage = FMath::GetMappedRangeValueClamped(LandedDamageVelocity, LandedDamage, FallVelocityZ);
    TakeDamage(FallDamage, FPointDamageEvent{}, nullptr, nullptr);
    UE_LOG(LogBaseCharacter, Display, TEXT("Player %s received landed damage: %f"), *GetName(), FallDamage);
}

// Called every frame
void ASTUBaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Replicate view rotation to server so other clients see correct weapon aim
    if (IsLocallyControlled() && IsPlayerControlled() && GetController())
    {
        const FRotator CurrentView = GetControlRotation();
        if (!ReplicatedViewRotation.Equals(CurrentView, 0.5f))
        {
            ServerUpdateViewRotation(CurrentView);
        }
    }
}

void ASTUBaseCharacter::TurnOff() 
{
    WeaponComponent->StopFire();
    WeaponComponent->Zoom(false);
    Super::TurnOff();
}

void ASTUBaseCharacter::Reset() 
{
    WeaponComponent->StopFire();
    WeaponComponent->Zoom(false);
    Super::Reset();
}

bool ASTUBaseCharacter::IsRunning() const
{
    return false;
}

float ASTUBaseCharacter::GetMovementDirection() const
{
    // If the speed is zero, return 0.0f since there is no movement
    if (GetVelocity().IsZero()) return 0.0f;

    // define the normalized velocity vector
    const auto VelocityNormal = GetVelocity().GetSafeNormal();
    // define variable for angle (scalar product) between GetActorForwardVector and VelocityNormal, return Radians
    const auto AngleBetween = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));
    // define variable (vector product) to calculate vector product
    const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);
    // Convert the angle from radians to degrees
    const auto Degrees = FMath::RadiansToDegrees(AngleBetween);

    // Return the angle in degrees multiplied by the sign of the Z component of the vector product,
    // if the vector product is not zero; otherwise we return the angle in degrees without changing
    return CrossProduct.IsZero() ? Degrees : Degrees * FMath::Sign(CrossProduct.Z);
}

void ASTUBaseCharacter::PlayPlayerReloadSound()
{
    if (PlayerReloadSound && GetNetMode() != NM_DedicatedServer)
    {
        UGameplayStatics::SpawnSoundAttached(PlayerReloadSound, GetRootComponent());
    }
}

void ASTUBaseCharacter::SetPlayerColor(const FLinearColor& Color)
{
    TeamColor = Color;
    if (GetMesh())
    {
        const auto MaterialInst = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
        if (MaterialInst)
        {
            MaterialInst->SetVectorParameterValue(MaterialColorName, Color);
        }
    }
}

void ASTUBaseCharacter::OnDeath()
{
    UE_LOG(LogBaseCharacter, Display, TEXT("Player %s is dead"), *GetName());

    // PlayAnimMontage(DeathAnimMontage);

    GetCharacterMovement()->DisableMovement();

    SetLifeSpan(LifeSpanOnDeath);

    // Disable capsule collision after death
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

    WeaponComponent->StopFire();
    WeaponComponent->Zoom(false);

    // Ragdoll Physics
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(true);

    if (HasAuthority())
    {
        MulticastPlayPlayerDeathSound();
    }

    UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathBodySound, GetActorLocation());
}

bool ASTUBaseCharacter::ServerTryPickupActor_Validate(AActor* PickupActor)
{
    return PickupActor != nullptr;
}

void ASTUBaseCharacter::ServerTryPickupActor_Implementation(AActor* PickupActor)
{
    ASTUBasePickup* Pickup = Cast<ASTUBasePickup>(PickupActor);
    if (!Pickup || !GetWorld()) return;

    static constexpr float MaxUseDistCm = 400.f;
    if (FVector::DistSquared(GetActorLocation(), Pickup->GetActorLocation()) > FMath::Square(MaxUseDistCm))
    {
        return;
    }

    Pickup->AuthorityTryGiveToPawn(this);
}