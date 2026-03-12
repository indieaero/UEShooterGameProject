// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "STUStaminaComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnStaminaDepletedSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTTHEMUP_API USTUStaminaComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USTUStaminaComponent();

    /** Returns true if the owner can run (stamina > 0 and not in recovery delay). */
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool CanRun() const;

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetStaminaPercent() const { return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 1.0f; }

    UFUNCTION(BlueprintCallable, Category = "Stamina")
    float GetCurrentStamina() const { return CurrentStamina; }

    /** True when stamina bar should be visible (running or recovering). */
    UFUNCTION(BlueprintCallable, Category = "Stamina")
    bool ShouldShowStaminaBar() const;

    /** Called each frame by owner. bWantsToRunAndMoving = WantsToRun && moving forward. */
    void UpdateStamina(float DeltaTime, bool bWantsToRunAndMoving);

    FOnStaminaDepletedSignature OnStaminaDepleted;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Stamina",
        meta = (ClampMin = "1.0", ClampMax = "500.0"))
    float MaxStamina = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "0.1"))
    float StaminaDrainRate = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "0.1"))
    float StaminaRecoveryRate = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina", meta = (ClampMin = "0.0"))
    float StaminaRecoveryDelay = 5.0f;

    virtual void BeginPlay() override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina)
    float CurrentStamina = 0.0f;

    /** True during the delay after stamina hit 0, before recovery starts. */
    UPROPERTY(Replicated)
    bool bIsRecoveryDelayed = false;

    UFUNCTION()
    void OnRep_CurrentStamina();

    void DrainStamina(float DeltaTime);
    void UpdateRecovery(float DeltaTime);
    void SetCurrentStamina(float NewStamina);
    void OnRecoveryDelayComplete();

    FTimerHandle RecoveryDelayHandle;
};
