// Shoot Them Up Game, All Rights Reserved.

#include "Player/STUPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/InputComponent.h"
#include "Components/STUHealthComponent.h"
#include "Components/STUStaminaComponent.h"
#include "Engine/Scene.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Components/STUWeaponComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimMontage.h"
#include "Animations/AnimUtils.h"
#include "Animations/STUKickFinishedAnimNotify.h"

// Sets default values in constructor
ASTUPlayerCharacter::ASTUPlayerCharacter(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
    SpringArmComponent->SetupAttachment(GetRootComponent());
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->SocketOffset = FVector(0.0f, 100.0f, 80.0f);

    CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
    CameraComponent->SetupAttachment(SpringArmComponent);

    CameraCollisionComponent = CreateDefaultSubobject<USphereComponent>("CameraCollisionComponent");
    CameraCollisionComponent->SetupAttachment(CameraComponent);
    CameraCollisionComponent->SetSphereRadius(10.0f);
    CameraCollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

    StaminaComponent = CreateDefaultSubobject<USTUStaminaComponent>("StaminaComponent");
}

void ASTUPlayerCharacter::ServerSetRunning_Implementation(bool bNewRunning)
{
    WantsToRun = bNewRunning;
}

bool ASTUPlayerCharacter::CanKick() const
{
    if (!KickMontage) return false;
    if (KickAnimInProgress) return false;
    if (HealthComponent && HealthComponent->IsDead()) return false;

    const UCharacterMovementComponent* Move = GetCharacterMovement();
    if (!Move) return false;

    if (Move->IsFalling()) return true;

    if (!Move->IsMovingOnGround()) return false;
    if (IsRunning()) return false;
    if (WeaponComponent && WeaponComponent->IsBusy()) return false;

    const float Speed2D = FVector(GetVelocity().X, GetVelocity().Y, 0.0f).Size();
    return Speed2D <= KickIdleHorizontalSpeedThreshold;
}

void ASTUPlayerCharacter::TryKick()
{
    if (!CanKick()) return;

    if (HasAuthority())
    {
        StartKickMontage();
    }
    else
    {
        ServerTryKick();
    }
}

void ASTUPlayerCharacter::StartKickMontage()
{
    if (WeaponComponent)
    {
        WeaponComponent->StopFire();
    }

    KickAnimInProgress = true;
    PlayAnimMontage(KickMontage);
    MulticastPlayKickMontage();
}

void ASTUPlayerCharacter::ServerTryKick_Implementation()
{
    if (!CanKick()) return;
    StartKickMontage();
}

void ASTUPlayerCharacter::MulticastPlayKickMontage_Implementation()
{
    if (!HasAuthority() && KickMontage)
    {
        KickAnimInProgress = true;
        PlayAnimMontage(KickMontage);
    }
}

void ASTUPlayerCharacter::InitKickNotify()
{
    if (!KickMontage) return;

    if (USTUKickFinishedAnimNotify* Notify = AnimUtils::FindNotifyByClass<USTUKickFinishedAnimNotify>(KickMontage))
    {
        Notify->OnNotified.AddUObject(this, &ASTUPlayerCharacter::OnKickMontageFinished);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Kick montage '%s' has no STUKickFinishedAnimNotify — add it at the end of the montage in the editor."),
            *GetNameSafe(KickMontage));
    }
}

void ASTUPlayerCharacter::OnKickMontageFinished(USkeletalMeshComponent* MeshComp)
{
    if (!MeshComp || MeshComp != GetMesh()) return;
    KickAnimInProgress = false;
}

void ASTUPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitKickNotify();

    check(CameraCollisionComponent);

    CameraCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASTUPlayerCharacter::OnCameraCollisionBeginOverlap);
    CameraCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ASTUPlayerCharacter::OnCameraCollisionEndOverlap);

    if (StaminaComponent)
    {
        StaminaComponent->OnStaminaDepleted.AddUObject(this, &ASTUPlayerCharacter::OnStaminaDepleted);
    }

    // Add run blur post-process to camera
    if (CameraComponent && RunBlurMaterial)
    {
        FWeightedBlendable Blendable;
        Blendable.Object = RunBlurMaterial;
        Blendable.Weight = 0.0f;
        CameraComponent->PostProcessSettings.WeightedBlendables.Array.Add(Blendable);
        CameraComponent->PostProcessBlendWeight = 1.0f;
    }

    // Add death post-process to camera (weight set in OnDeath)
    if (CameraComponent && DeathPostProcessMaterial)
    {
        FWeightedBlendable Blendable;
        Blendable.Object = DeathPostProcessMaterial;
        Blendable.Weight = 0.0f;
        CameraComponent->PostProcessSettings.WeightedBlendables.Array.Add(Blendable);
        CameraComponent->PostProcessBlendWeight = 1.0f;
    }
}

void ASTUPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTUPlayerCharacter, WantsToRun);
}

// Called to bind functionality to input
void ASTUPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);
    check(WeaponComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ASTUPlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ASTUPlayerCharacter::MoveRight);
    PlayerInputComponent->BindAxis("LookUp", this, &ASTUPlayerCharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnAround", this, &ASTUPlayerCharacter::AddControllerYawInput);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASTUPlayerCharacter::CheckAndJump);
    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ASTUPlayerCharacter::OnStartRunning);
    PlayerInputComponent->BindAction("Run", IE_Released, this, &ASTUPlayerCharacter::OnStopRunning);

    //PlayerInputComponent->BindAction("Fire", IE_Pressed, WeaponComponent, &USTUWeaponComponent::StartFire);
    //PlayerInputComponent->BindAction("Fire", IE_Released, WeaponComponent, &USTUWeaponComponent::StopFire);
    PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, WeaponComponent, &USTUWeaponComponent::NextWeapon);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, WeaponComponent, &USTUWeaponComponent::Reload);
    PlayerInputComponent->BindAction("Kick", IE_Pressed, this, &ASTUPlayerCharacter::OnKickPressed);

    //DECLARE_DELEGATE_OneParam(FZoomInputSignature, bool);
    //PlayerInputComponent->BindAction<FZoomInputSignature>("Zoom", IE_Pressed, WeaponComponent, &USTUWeaponComponent::Zoom, true);
    //PlayerInputComponent->BindAction<FZoomInputSignature>("Zoom", IE_Released, WeaponComponent, &USTUWeaponComponent::Zoom, false);
}

void ASTUPlayerCharacter::MoveForward(float Amount)
{
    IsMovingForward = Amount > 0.0f;
    if (Amount == 0.0f) return;
    AddMovementInput(GetActorForwardVector(), Amount);
}

void ASTUPlayerCharacter::MoveRight(float Amount)
{
    // disable turning A and D while running
    if (IsRunning() || Amount == 0.0f) return;
    AddMovementInput(GetActorRightVector(), Amount);
}

void ASTUPlayerCharacter::CheckAndJump()
{
    if (IsRunning()) return;
    Jump();
}

void ASTUPlayerCharacter::OnKickPressed()
{
    TryKick();
}

void ASTUPlayerCharacter::OnStartRunning()
{
    if (StaminaComponent && !StaminaComponent->CanRun()) return;

    WantsToRun = true;
    ServerSetRunning(WantsToRun);
}

void ASTUPlayerCharacter::OnStopRunning()
{
    WantsToRun = false;
    ServerSetRunning(WantsToRun);
}

bool ASTUPlayerCharacter::IsRunning() const
{
    if (StaminaComponent && !StaminaComponent->CanRun()) return false;

    const FVector Velocity = GetVelocity();
    const float Speed = Velocity.Size();
    if (!WantsToRun || Speed < KINDA_SMALL_NUMBER) return false;

    // Run only when moving primarily forward (W)
    const float ForwardDot = FVector::DotProduct(Velocity.GetSafeNormal(), GetActorForwardVector());
    constexpr float MinForwardDot = 0.7f;  // 0 = any direction, 1 = strictly forward
    return ForwardDot >= MinForwardDot;
}

void ASTUPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update run blur post-process weight
    if (CameraComponent && RunBlurMaterial)
    {
        for (FWeightedBlendable& Blendable : CameraComponent->PostProcessSettings.WeightedBlendables.Array)
        {
            if (Blendable.Object == RunBlurMaterial)
            {
                Blendable.Weight = IsRunning() ? 1.0f : 0.0f;
                break;
            }
        }
    }

    if (StaminaComponent)
    {
        const FVector Velocity = GetVelocity();
        const float Speed = Velocity.Size();
        const bool bWantsToRunAndMoving = WantsToRun && Speed >= KINDA_SMALL_NUMBER;
        const float ForwardDot = bWantsToRunAndMoving
            ? FVector::DotProduct(Velocity.GetSafeNormal(), GetActorForwardVector())
            : 0.0f;
        constexpr float MinForwardDot = 0.7f;
        const bool bMovingForward = ForwardDot >= MinForwardDot;

        StaminaComponent->UpdateStamina(DeltaTime, bWantsToRunAndMoving && bMovingForward);
    }

    UpdateRunningSounds();
}

void ASTUPlayerCharacter::UpdateRunningSounds()
{
    if (GetNetMode() == NM_DedicatedServer) return;
    if (HealthComponent && HealthComponent->IsDead()) return;

    const bool bRunning = IsRunning();
    const bool bTired = StaminaComponent && StaminaComponent->IsInRecoveryDelay();

    if (bRunning)
    {
        if (!RunningVoiceComponent && PlayerVoiceRunning)
        {
            RunningVoiceComponent = UGameplayStatics::SpawnSoundAttached(PlayerVoiceRunning, GetRootComponent());
        }
        if (TiredVoiceComponent)
        {
            TiredVoiceComponent->Stop();
            TiredVoiceComponent = nullptr;
        }
    }
    else if (bTired)
    {
        if (!TiredVoiceComponent && PlayerTiredRunning)
        {
            TiredVoiceComponent = UGameplayStatics::SpawnSoundAttached(PlayerTiredRunning, GetRootComponent());
        }
        if (RunningVoiceComponent)
        {
            RunningVoiceComponent->Stop();
            RunningVoiceComponent = nullptr;
        }
    }
    else
    {
        if (RunningVoiceComponent)
        {
            RunningVoiceComponent->Stop();
            RunningVoiceComponent = nullptr;
        }
        if (TiredVoiceComponent)
        {
            TiredVoiceComponent->Stop();
            TiredVoiceComponent = nullptr;
        }
    }
}

void ASTUPlayerCharacter::OnStaminaDepleted()
{
    WantsToRun = false;
    ServerSetRunning(false);
}

void ASTUPlayerCharacter::OnDeath()
{
    Super::OnDeath();

    KickAnimInProgress = false;

    // Apply death post-process to camera
    if (CameraComponent && DeathPostProcessMaterial)
    {
        for (FWeightedBlendable& Blendable : CameraComponent->PostProcessSettings.WeightedBlendables.Array)
        {
            if (Blendable.Object == DeathPostProcessMaterial)
            {
                Blendable.Weight = 1.0f;
                break;
            }
        }
    }

    //if (Controller) Controller->ChangeState(NAME_Spectating);
}

void ASTUPlayerCharacter::OnCameraCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    CheckCameraOverlap();
}

void ASTUPlayerCharacter::OnCameraCollisionEndOverlap(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    CheckCameraOverlap();
}

void ASTUPlayerCharacter::CheckCameraOverlap()
{
    // Hide character mesh if camera is colliding with capsule
    const auto HideMesh = CameraCollisionComponent->IsOverlappingComponent(GetCapsuleComponent());
    GetMesh()->SetOwnerNoSee(HideMesh);

    // Hide all weapon mesh too
    TArray<USceneComponent*> MeshChildren;
    GetMesh()->GetChildrenComponents(true, MeshChildren);

    for (auto MeshChild : MeshChildren)
    {
        const auto MeshChildGeometry = Cast<UPrimitiveComponent>(MeshChild);
        if (MeshChildGeometry)
        {
            MeshChildGeometry->SetOwnerNoSee(HideMesh);
        }
    }
}
