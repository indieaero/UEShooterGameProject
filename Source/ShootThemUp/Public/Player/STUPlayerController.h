// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "STUCoreTypes.h"
#include "STUPlayerController.generated.h"

class USTURespawnComponent;

UCLASS()
class SHOOTTHEMUP_API ASTUPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ASTUPlayerController();

    UFUNCTION(Client, Reliable)
    void ClientStartRespawnTimer(int32 RespawnTime);

    UFUNCTION(Server, Reliable)
    void ServerSetPause();

    // Cycle between players while spectating. Can also be called from BP/UI
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void SpectateNext();

    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void SpectatePrev();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USTURespawnComponent* RespawnComponent;

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    void OnPauseGame();
    void OnMatchStateChanged(ESTUMatchState State);
    void OnMuteSound();

    // Current index into GameState PlayerList used for spectating
    UPROPERTY(Replicated)
    int32 CurrentSpectateIndex = INDEX_NONE;

    UFUNCTION(Server, Reliable)
    void ServerSpectateNext();

    UFUNCTION(Server, Reliable)
    void ServerSpectatePrev();

    void SpectateOffset(int32 Offset);
};
