// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "STUCoreTypes.h"
#include "STUHealthComponent.generated.h"

class UCameraShakeBase;
class UPhisicalMaterial;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTTHEMUP_API USTUHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Sets default values for this component's properties
    USTUHealthComponent();

    // Delegates
    FOnDeathSignature OnDeath;
    FOnHealthChangedSignature OnHealthChanged;

    UFUNCTION(BlueprintCallable, Category = "Health")
    bool IsDead() const { return FMath::IsNearlyZero(Health); }

    UFUNCTION(BlueprintCallable, Category = "Health")
    float GetHealthPercent() const { return Health / MaxHealth; }

    // Create a function for the Health variable that will return its value
    float GetHealth() const { return Health; }

    bool TryToAddHealth(float HealthAmount);

protected:
    // Create max health variable and can change it in Blueprints
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Heal")
    bool AutoHeal = true;

    // EditCondition is bool, Heal parameters only available if AutoHeal is true
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Heal", meta = (EditCondition = "AutoHeal"))
    float HealUpdateTime = 0.3f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Heal", meta = (EditCondition = "AutoHeal"))
    float HealDelay = 3.0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Heal", meta = (EditCondition = "AutoHeal"))
    float HealModifier = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX");
    TSubclassOf<UCameraShakeBase> CameraShake;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health")
    TMap<UPhysicalMaterial*, float> DamageModifiers;

    // Register replication of properties (Health, MaxHealth)
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Called when the game starts
    virtual void BeginPlay() override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_Health)
    float Health = 0.0f;

    // To avoid multiple replication of Health value
    float LastReplicatedHealth = -1.0f;

    // RepNotify for changes made to Health.
    UFUNCTION()
    void OnRep_Health();

    FTimerHandle HealTimerHandle;

    // TODO: read and watch about delegates UE !
    UFUNCTION()
    void OnTakeAnyDamage(
        AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void OnTakePointDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation,
        class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType,
        AActor* DamageCauser);

    UFUNCTION()
    void OnTakeRadialDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, FVector Origin,
        const FHitResult& HitInfo, class AController* InstigatedBy, AActor* DamageCauser);

    void HealUpdate();
    void SetHealth(float NewHealth);
    void PlayCameraShake();

    void Killed(AController* KillerController);
    void ApplyDamage(float Damage, AController* InstigatedBy);
    float GetPointDamageModifier(AActor* DamagedActor, const FName& BoneName) const;

    void ReportDamageEvent(float Damage, AController* InstigatedBy);
};
