// Shoot Them Up Game, All Rights Reserved.

#include "Player/STUPlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogSTUPlayerState, All, All);

void ASTUPlayerState::LogInfo() const 
{
    UE_LOG(LogSTUPlayerState, Display, TEXT("TeamID: %d, Kills: %d, Deaths: %d"), TeamID, KillsNum, DeathsNum);
}
