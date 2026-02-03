// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "STUPlayerState.generated.h"

UCLASS()
class SHOOTTHEMUP_API ASTUPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    void SetTeamID(int32 ID) { TeamID = ID; }
    int32 GetTeamID() const { return TeamID; }

    void SetTeamColor(const FLinearColor& Color) { TeamColor = Color; }
    FLinearColor GetTeamColor() const { return TeamColor; }

    void AddKill() { ++KillsNum; }
    int32 GetKillsNum() const { return KillsNum; }

    void AddDeath() { ++DeathsNum; }
    int32 GetDeathsNum() const { return DeathsNum; }

    void LogInfo() const;

protected:
    // Registering replicated properties (TeamID, TeamColor, KillsNum, DeathsNum)
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // Must have UPROPERTY(Replicated) so DOREPLIFETIME can find them in reflection.
    UPROPERTY(Replicated)
    int32 TeamID = 0;

    UPROPERTY(Replicated)
    FLinearColor TeamColor;

    UPROPERTY(Replicated)
    int32 KillsNum = 0;

    UPROPERTY(Replicated)
    int32 DeathsNum = 0;
};
