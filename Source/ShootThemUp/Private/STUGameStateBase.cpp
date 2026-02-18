// Shoot Them Up Game, All Rights Reserved.

#include "STUGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Player/STUPlayerCharacter.h"

ASTUGameStateBase::ASTUGameStateBase()
{
}

void ASTUGameStateBase::SetMatchState(ESTUMatchState NewState)
{
    if (MatchState == NewState) return;
    MatchState = NewState;
    OnMatchStateChanged.Broadcast(MatchState);
}

void ASTUGameStateBase::SetCurrentRound(int32 NewRound)
{
    CurrentRound = NewRound;
}

void ASTUGameStateBase::SetRoundCountDown(int32 NewRoundCountDown)
{
    RoundCountDown = NewRoundCountDown;
}

void ASTUGameStateBase::SetGameData(const FGameData& NewGameData)
{
    GameData = NewGameData;
}

void ASTUGameStateBase::AddPlayer(ASTUPlayerCharacter* Player) 
{
    if (!HasAuthority() || !Player)
    {
        return;
    }

    PlayerList.AddUnique(Player);
}

void ASTUGameStateBase::RemovePlayer(ASTUPlayerCharacter* Player)
{
    if (!HasAuthority() || !Player)
    {
        return;
    }

    PlayerList.Remove(Player);
}

// Replication Notification Callbacks
void ASTUGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASTUGameStateBase, MatchState);
    DOREPLIFETIME(ASTUGameStateBase, CurrentRound);
    DOREPLIFETIME(ASTUGameStateBase, RoundCountDown);
    DOREPLIFETIME(ASTUGameStateBase, GameData);
    DOREPLIFETIME(ASTUGameStateBase, PlayerList);
}

void ASTUGameStateBase::OnRep_MatchState()
{
    OnMatchStateChanged.Broadcast(MatchState);
}