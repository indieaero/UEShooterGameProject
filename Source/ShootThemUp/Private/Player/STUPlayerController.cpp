// Shoot Them Up Game, All Rights Reserved.

#include "Player/STUPlayerController.h"
#include "Components/STURespawnComponent.h"
#include "STUGameStateBase.h"
#include "STUGameInstance.h"

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

    OnNewPawn.Broadcast(InPawn);
}

void ASTUPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;

    InputComponent->BindAction("PauseGame", IE_Pressed, this, &ASTUPlayerController::OnPauseGame);
    InputComponent->BindAction("Mute", IE_Pressed, this, &ASTUPlayerController::OnMuteSound);
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
