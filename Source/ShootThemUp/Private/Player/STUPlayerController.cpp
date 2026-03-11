// Shoot Them Up Game, All Rights Reserved.

#include "Player/STUPlayerController.h"
#include "Components/STURespawnComponent.h"
#include "STUGameStateBase.h"
#include "STUGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Player/STUPlayerCharacter.h"
#include "Player/STUBaseCharacter.h"
#include "Components/STUHealthComponent.h"
#include "Components/STUWeaponComponent.h"
#include "STUUtils.h"

ASTUPlayerController::ASTUPlayerController()
{
    RespawnComponent = CreateDefaultSubobject<USTURespawnComponent>("RespawnComponent");
}

void ASTUPlayerController::ClientStartRespawnTimer_Implementation(int32 RespawnTime) 
{
    if (RespawnComponent)
    {
        RespawnComponent->Respawn(RespawnTime);
    }
}

void ASTUPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        const auto GameState = GetWorld()->GetGameState<ASTUGameStateBase>();
        if (GameState)
        {
            GameState->OnMatchStateChanged.AddUObject(this, &ASTUPlayerController::OnMatchStateChanged);
        }
    }
}

void ASTUPlayerController::OnMatchStateChanged(ESTUMatchState State)
{
    if (State == ESTUMatchState::InProgress)
    {
        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
    }
    else
    {
        SetInputMode(FInputModeUIOnly());
        bShowMouseCursor = true;
    }
}

void ASTUPlayerController::OnMuteSound() 
{
    if (!GetWorld()) return;

    const auto STUGameInstance = GetWorld()->GetGameInstance<USTUGameInstance>();
    if (!STUGameInstance) return;

    STUGameInstance->ToggleVolume();
}

void ASTUPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Initial aim horizontal (pitch 0) so default pose is not "weapon up"
    if (InPawn)
    {
        const FRotator HorizontalView(0.f, InPawn->GetActorRotation().Yaw, 0.f);
        SetControlRotation(HorizontalView);
    }

    // Reset recoil state on new pawn
    RecoilTargetPitch = 0.0f;
    RecoilTargetYaw = 0.0f;
    RecoilCurrentPitch = 0.0f;
    RecoilCurrentYaw = 0.0f;

    OnNewPawn.Broadcast(InPawn);
}

void ASTUPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;

    InputComponent->BindAction("PauseGame", IE_Pressed, this, &ASTUPlayerController::OnPauseGame);
    InputComponent->BindAction("Mute", IE_Pressed, this, &ASTUPlayerController::OnMuteSound);

    // spectate bindings on other buttons
    InputComponent->BindAction("SpectateNext", IE_Pressed, this, &ASTUPlayerController::SpectateNext);
    InputComponent->BindAction("SpectatePrev", IE_Pressed, this, &ASTUPlayerController::SpectatePrev);

    // Fire & Zoom bindings
    InputComponent->BindAction("Fire", IE_Pressed, this, &ASTUPlayerController::OnFirePressed);
    InputComponent->BindAction("Fire", IE_Released, this, &ASTUPlayerController::OnFireReleased);
    InputComponent->BindAction("Zoom", IE_Pressed, this, &ASTUPlayerController::OnZoomPressed);
    InputComponent->BindAction("Zoom", IE_Released, this, &ASTUPlayerController::OnZoomReleased);
}

void ASTUPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateRecoil(DeltaSeconds);
}

void ASTUPlayerController::OnPauseGame()
{
    ServerSetPause();  // Pause is server-only (GameMode lives on server)
}

void ASTUPlayerController::ServerSetPause_Implementation()
{
    if (!GetWorld() || !GetWorld()->GetAuthGameMode()) return;
    GetWorld()->GetAuthGameMode()->SetPause(this);
}

void ASTUPlayerController::SpectateNext()
{
    ServerSpectateNext();
}

void ASTUPlayerController::SpectatePrev()
{
    ServerSpectatePrev();
}

void ASTUPlayerController::ServerSpectateNext_Implementation()
{
    auto PlayerPawn = GetPawn();
    if (PlayerPawn)
    {
        auto HealthComponent = STUUtils::GetSTUPlayerComponent<USTUHealthComponent>(PlayerPawn);
        if (HealthComponent && !HealthComponent->IsDead())
        {
            return;  // Only allow spectating when dead
        }
    }

    SpectateOffset(+1);
}

void ASTUPlayerController::ServerSpectatePrev_Implementation()
{
    auto PlayerPawn = GetPawn();
    if (PlayerPawn)
    {
        auto HealthComponent = STUUtils::GetSTUPlayerComponent<USTUHealthComponent>(PlayerPawn);
        if (HealthComponent && !HealthComponent->IsDead())
        {
            return;  // Only allow spectating when dead
        }
    }

    SpectateOffset(-1);
}

void ASTUPlayerController::SpectateOffset(int32 Offset)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!GetWorld())
    {
        return;
    }

    ASTUGameStateBase* STUGameState = GetWorld()->GetGameState<ASTUGameStateBase>();
    if (!STUGameState)
    {
        return;
    }

    const TArray<ASTUPlayerCharacter*>& PlayerList = STUGameState->GetPlayerList();
    const int32 NumPlayers = PlayerList.Num();
    if (NumPlayers == 0)
    {
        return;
    }

    if (CurrentSpectateIndex == INDEX_NONE)
    {
        // Try to find current pawn in list; if not found, start from zero.
        int32 FoundIndex = INDEX_NONE;
        if (APawn* CurrentPawn = GetPawn())
        {
            FoundIndex = PlayerList.IndexOfByKey(CurrentPawn);
        }
        CurrentSpectateIndex = (FoundIndex != INDEX_NONE) ? FoundIndex : 0;
    }
    // Cycle through list with offset, wrapping around using modulo.
    CurrentSpectateIndex = (CurrentSpectateIndex + Offset + NumPlayers) % NumPlayers;

    if (ASTUPlayerCharacter* NewTarget = PlayerList[CurrentSpectateIndex])
    {
        SetViewTargetWithBlend(NewTarget, 0.0f);
    }
}

void ASTUPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASTUPlayerController, CurrentSpectateIndex);
}

bool ASTUPlayerController::CanSpectate() const
{
    // can spectate if no pawn (after death) or pawn is dead
    APawn* PlayerPawn = GetPawn();
    if (!PlayerPawn) return true;
    const auto HealthComponent = STUUtils::GetSTUPlayerComponent<USTUHealthComponent>(PlayerPawn);
    return !HealthComponent || HealthComponent->IsDead();
}

void ASTUPlayerController::OnFirePressed()
{
    if (CanSpectate())
    {
        SpectateNext();  
        return;
    }
    if (ASTUBaseCharacter* BaseChar = Cast<ASTUBaseCharacter>(GetPawn()))
    {
        if (USTUWeaponComponent* WC = BaseChar->GetWeaponComponent()) 
            WC->StartFire();
    }
}

void ASTUPlayerController::OnFireReleased()
{
    if (CanSpectate()) return;
    if (ASTUBaseCharacter* BaseChar = Cast<ASTUBaseCharacter>(GetPawn()))
    {
        if (USTUWeaponComponent* WC = BaseChar->GetWeaponComponent()) 
            WC->StopFire();
    }
}

void ASTUPlayerController::OnZoomPressed()
{
    if (CanSpectate())
    {
        SpectatePrev();
        return;
    }
    if (ASTUBaseCharacter* BaseChar = Cast<ASTUBaseCharacter>(GetPawn()))
    {
        if (USTUWeaponComponent* WC = BaseChar->GetWeaponComponent()) 
            WC->Zoom(true);

        if (ASTUPlayerCharacter* PlayerChar = Cast<ASTUPlayerCharacter>(BaseChar))
        {
            PlayerChar->OnZoomStateChanged(true);
        }
    }
}

void ASTUPlayerController::OnZoomReleased()
{
    if (CanSpectate()) return;
    if (ASTUBaseCharacter* BaseChar = Cast<ASTUBaseCharacter>(GetPawn()))
    {
        if (USTUWeaponComponent* WC = BaseChar->GetWeaponComponent()) 
            WC->Zoom(false);

        if (ASTUPlayerCharacter* PlayerChar = Cast<ASTUPlayerCharacter>(BaseChar))
        {
            PlayerChar->OnZoomStateChanged(false);
        }
    }
}

void ASTUPlayerController::UpdateRecoil(float DeltaSeconds)
{
    if (!IsLocalController()) return;
    if (CanSpectate()) return;

    // Move current recoil toward target smoothly
    const float NewPitch = FMath::FInterpTo(RecoilCurrentPitch, RecoilTargetPitch, DeltaSeconds, RecoilInterpSpeed);
    const float NewYaw = FMath::FInterpTo(RecoilCurrentYaw, RecoilTargetYaw, DeltaSeconds, RecoilInterpSpeed);

    const float DeltaPitch = NewPitch - RecoilCurrentPitch;
    const float DeltaYaw = NewYaw - RecoilCurrentYaw;

    if (!FMath::IsNearlyZero(DeltaPitch) || !FMath::IsNearlyZero(DeltaYaw))
    {
        FRotator ControlRot = GetControlRotation();

        // Apply only the incremental recoil delta this frame
        ControlRot.Pitch -= DeltaPitch;  // positive target pitch means looking higher
        ControlRot.Yaw += DeltaYaw;

        SetControlRotation(ControlRot);

        RecoilCurrentPitch = NewPitch;
        RecoilCurrentYaw = NewYaw;
    }
}

void ASTUPlayerController::ApplyRecoil(float PitchAmount, float YawAmount) 
{
    if (CanSpectate()) return;
    //only local player
    if (!IsLocalController()) return;

    // Increase target recoil; Tick will smoothly apply it to control rotation
    RecoilTargetPitch += PitchAmount;
    RecoilTargetYaw += YawAmount;
}
