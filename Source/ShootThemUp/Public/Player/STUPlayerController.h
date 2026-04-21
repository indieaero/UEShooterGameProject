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

    /** Sends pending display name from USTUGameInstance (or default) to the server. */
    void ApplyPendingDisplayNameFromSettings();

    // Cycle between players while spectating. Can also be called from BP/UI
    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void SpectateNext();

    UFUNCTION(BlueprintCallable, Category = "Spectator")
    void SpectatePrev();

    void ApplyRecoil(float PitchAmount, float YawAmount);

    // Smooth recoil update each frame
    virtual void Tick(float DeltaSeconds) override;

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

    bool CanSpectate() const;

    void OnFirePressed();
    void OnFireReleased();
    void OnZoomPressed();
    void OnZoomReleased();

    void UpdateRecoil(float DeltaSeconds);

    // Current index into GameState PlayerList used for spectating
    UPROPERTY(Replicated)
    int32 CurrentSpectateIndex = INDEX_NONE;

    UFUNCTION(Server, Reliable)
    void ServerSpectateNext();

    UFUNCTION(Server, Reliable)
    void ServerSpectatePrev();

    UFUNCTION(Server, Reliable)
    void ServerSetPlayerDisplayName(const FString& Name);

    void SpectateOffset(int32 Offset);

    // How fast current recoil moves toward target (degrees per second)
    UPROPERTY(EditDefaultsOnly, Category = "Recoil")
    float RecoilInterpSpeed = 20.0f;

    // Target recoil offset accumulated from shots
    float RecoilTargetPitch = 0.0f;
    float RecoilTargetYaw = 0.0f;

    // Current recoil offset already applied to control rotation
    float RecoilCurrentPitch = 0.0f;
    float RecoilCurrentYaw = 0.0f;
};
