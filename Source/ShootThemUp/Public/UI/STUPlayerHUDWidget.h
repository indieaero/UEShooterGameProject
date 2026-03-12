// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "UI/STUBaseWidget.h"
#include "STUCoreTypes.h"
#include "STUPlayerHUDWidget.generated.h"

class UProgressBar;
class USTUHealthComponent;
class USTUStaminaComponent;

UCLASS()
class SHOOTTHEMUP_API USTUPlayerHUDWidget : public USTUBaseWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool ShouldShowStaminaBar() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool GetCurrentWeaponUIData(FWeaponUIData& UIData) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool IsPlayerAlive() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool IsPlayerSpectating() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool IsRespawnInProgress() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnTakeDamage();

    UFUNCTION(BlueprintCallable, Category = "UI")
    int32 GetKillsNum() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    FString FormatBullets(int32 BulletsNum) const;

protected:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthProgressBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* StaminaProgressBar;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    float PercentColorThreshold = 0.3f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    FLinearColor GoodColor = FLinearColor::Green;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    FLinearColor BadColor = FLinearColor::Red;

    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

private:
    void OnNewPawn(APawn* NewPawn);
    void UpdateHealthBar();
    void UpdateStaminaBar();

    // Callback for OnClientDamageTaken of HealthComponent. call OnTakeDamage() (damage image) only when the server explicitly
    // reported damage through Client RPC, and not on any drop of health percentage in UpdateHealthBar
    void OnDamageTakenForHUD();

    FTimerHandle HealthBarRefreshHandle;
    float LastKnownHealth = -1.0f;

    // store the component and the subscription handle for OnClientDamageTaken
    TWeakObjectPtr<USTUHealthComponent> CachedHealthComponent;
    FDelegateHandle DamageTakenHandle;
};
