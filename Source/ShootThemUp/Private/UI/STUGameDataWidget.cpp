// Shoot Them Up Game, All Rights Reserved.

#include "UI/STUGameDataWidget.h"
#include "STUGameStateBase.h"
#include "STUPlayerState.h"

int32 USTUGameDataWidget::GetCurrentRoundNum() const
{
    const auto GameState = GetSTUGameState();
    return GameState ? GameState->GetCurrentRound() : 0;
}

int32 USTUGameDataWidget::GetTotalRoundsNum() const
{
    const auto GameState = GetSTUGameState();
    return GameState ? GameState->GetGameData().RoundsNum : 0;
}

int32 USTUGameDataWidget::GetRoundSecondsRemaining() const 
{
    const auto GameState = GetSTUGameState();
    return GameState ? GameState->GetRoundCountDown() : 0;
}

ASTUGameStateBase* USTUGameDataWidget::GetSTUGameState() const
{
    return GetWorld() ? GetWorld()->GetGameState<ASTUGameStateBase>() : nullptr;
}
ASTUPlayerState* USTUGameDataWidget::GetSTUPlayerState() const 
{
    return GetOwningPlayer() ? Cast<ASTUPlayerState>(GetOwningPlayer()->PlayerState) : nullptr;
}