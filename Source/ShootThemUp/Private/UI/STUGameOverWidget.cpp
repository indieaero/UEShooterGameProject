// Shoot Them Up Game, All Rights Reserved.

#include "UI/STUGameOverWidget.h"
#include "STUGameStateBase.h"
#include "Player/STUPlayerState.h"
#include "UI/STUPlayerStatRowWidget.h"
#include "Components/VerticalBox.h"
#include "STUUtils.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USTUGameOverWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (GetWorld())
    {
        const auto GameState = GetWorld()->GetGameState<ASTUGameStateBase>();
        if (GameState)
        {
            GameState->OnMatchStateChanged.AddUObject(this, &USTUGameOverWidget::OnMatchStateChanged);
        }
    }

    if (ResetLevelButton)
    {
        ResetLevelButton->OnClicked.AddDynamic(this, &USTUGameOverWidget::OnResetLevel);
    }
}

void USTUGameOverWidget::OnMatchStateChanged(ESTUMatchState State)
{
    if(State == ESTUMatchState::GameOver)
    {
        UpdatePlayersStat();
    }
}

void USTUGameOverWidget::UpdatePlayersStat() 
{
    if (!GetWorld() || !PlayerStatBox) return;

    const auto GameState = GetWorld()->GetGameState<ASTUGameStateBase>();
    if (!GameState) return;

    PlayerStatBox->ClearChildren();

    for (int32 i = 0; i < GameState->PlayerArray.Num(); ++i)
    {
        const auto PlayerState = Cast<ASTUPlayerState>(GameState->PlayerArray[i]);
        if (!PlayerState) continue;

        const auto PlayerStatRowWidget = CreateWidget<USTUPlayerStatRowWidget>(GetWorld(), PlayerStatRowWidgetClass);
        if (!PlayerStatRowWidget) continue;

        PlayerStatRowWidget->SetPlayerName(FText::FromString(PlayerState->GetPlayerName()));
        PlayerStatRowWidget->SetKills(STUUtils::TextFromInt(PlayerState->GetKillsNum()));
        PlayerStatRowWidget->SetDeaths(STUUtils::TextFromInt(PlayerState->GetDeathsNum()));
        PlayerStatRowWidget->SetTeam(STUUtils::TextFromInt(PlayerState->GetTeamID()));

        // local player is the one whose PlayerState belongs to our controller.
        const bool bIsLocalPlayer = (GetOwningPlayer() && PlayerState->GetOwningController() == GetOwningPlayer());
        PlayerStatRowWidget->SetPlayerIndocatorVisibility(bIsLocalPlayer);
        PlayerStatRowWidget->SetTeamColor(PlayerState->GetTeamColor());

        PlayerStatBox->AddChild(PlayerStatRowWidget);
    }
}

void USTUGameOverWidget::OnResetLevel() 
{
    //const FName CurrentLevelName = "TestLevel";
    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    UGameplayStatics::OpenLevel(this, FName(CurrentLevelName));
}
