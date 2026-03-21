// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "STUCoreTypes.h"
#include "STUGameStateBase.generated.h"

class ASTUPlayerCharacter;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnKillFeedUpdatedSignature, const FString&, const FString&);

UCLASS()
class SHOOTTHEMUP_API ASTUGameStateBase : public AGameStateBase
{
    GENERATED_BODY()
public:
    ASTUGameStateBase();

    UPROPERTY(ReplicatedUsing = OnRep_MatchState, BlueprintReadOnly, Category = "Game")
    ESTUMatchState MatchState = ESTUMatchState::WaitingToStart;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 CurrentRound = 1;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 RoundCountDown = 0;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    FGameData GameData;

    UPROPERTY(ReplicatedUsing = OnRep_KillFeed, BlueprintReadOnly, Category = "Game")
    TArray<FKillfeedEntry> KillFeedEntries;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    TArray<TObjectPtr<ASTUPlayerCharacter>> PlayerList;

    UFUNCTION(BlueprintCallable, Category = "Game")
    const TArray<ASTUPlayerCharacter*>& GetPlayerList() const { return PlayerList; }

    FOnMatchStateChangedSignature OnMatchStateChanged;

    FOnKillFeedUpdatedSignature OnKillFeedUpdated;

    UFUNCTION(BlueprintCallable, Category = "Game")
    ESTUMatchState GetMatchState() const { return MatchState; }

    UFUNCTION(BlueprintCallable, Category = "Game")
    int32 GetCurrentRound() const { return CurrentRound; }

    UFUNCTION(BlueprintCallable, Category = "Game")
    int32 GetRoundCountDown() const { return RoundCountDown; }

    UFUNCTION(BlueprintCallable, Category = "Game")
    const FGameData& GetGameData() const { return GameData; }

    void SetMatchState(ESTUMatchState NewState);

    void SetCurrentRound(int32 NewRound);

    void SetRoundCountDown(int32 NewRoundCountDown);

    void SetGameData(const FGameData& NewGameData);

    // Add/remove players on the server; list is AddUnique to avoid duplicates
    void AddPlayer(ASTUPlayerCharacter* Player);
    void RemovePlayer(ASTUPlayerCharacter* Player);

    void ReportKill(const FString& KillerName, const FString& VictimName, UTexture2D* WeaponIcon = nullptr);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_MatchState();

    UFUNCTION()
    void OnRep_KillFeed();

private:
    int32 LastReplicatedKillCount = 0;
};
