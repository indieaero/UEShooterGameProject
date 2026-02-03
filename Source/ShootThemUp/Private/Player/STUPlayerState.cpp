// Shoot Them Up Game, All Rights Reserved.

#include "Player/STUPlayerState.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(LogSTUPlayerState, All, All);

void ASTUPlayerState::LogInfo() const 
{
    UE_LOG(LogSTUPlayerState, Display, TEXT("TeamID: %d, Kills: %d, Deaths: %d"), TeamID, KillsNum, DeathsNum);
}

void ASTUPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
    // Replicated properties: the engine replicates TeamID, TeamColor, KillsNum, DeathsNum
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASTUPlayerState, TeamID);
    DOREPLIFETIME(ASTUPlayerState, TeamColor);
    DOREPLIFETIME(ASTUPlayerState, KillsNum);
    DOREPLIFETIME(ASTUPlayerState, DeathsNum);
}
