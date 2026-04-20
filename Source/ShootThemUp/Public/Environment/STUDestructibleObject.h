// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STUDestructibleObject.generated.h"

class UGeometryCollectionComponent;
class UFieldSystemComponent;

/**
 * Replicated destructible object with Health.
 * Invulnerable until Health reaches 0, then applies Chaos Strain Field on server and all clients.
 * Any weapon dealing damage will destroy it - no per-weapon logic needed.
 * Use as parent for BP_Object - add Geometry Collection as child in Blueprint.
 */
UCLASS()
class SHOOTTHEMUP_API ASTUDestructibleObject : public AActor
{
    GENERATED_BODY()

public:
    ASTUDestructibleObject();

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
        AActor* DamageCauser) override;

    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetHealth() const
    {
        return Health;
    }

    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool IsDestroyed() const
    {
        return bIsDestroyed;
    }

    /** Call from Blueprint to trigger destruction at custom position (e.g. overlap, custom logic) */
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestructionAt(FVector Location);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UFieldSystemComponent* FieldSystemComponent;

    /** Auto-found or assign in Blueprint. Named Ref to avoid conflict with BP component. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    UGeometryCollectionComponent* GeometryCollectionRef;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction")
    float MaxHealth = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Destruction")
    float Health = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsDestroyed, BlueprintReadOnly, Category = "Destruction")
    bool bIsDestroyed = false;

    /** Strain field radius - must cover the entire Geometry Collection */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "1.0"))
    float StrainRadius = 200.0f;

    /** Strain magnitude - higher = more violent destruction */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "1.0"))
    float StrainMagnitude = 1000000.0f;

    /** Strain field iterations - higher = breaks deeper into cluster hierarchy */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "1", ClampMax = "10"))
    int32 StrainIterations = 1;

    /** If true, use last hit point for field position. If false, use actor center. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction")
    bool bUseLastHitPoint = false;

    UFUNCTION()
    void OnRep_Health();

    UFUNCTION()
    void OnRep_IsDestroyed();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastApplyDestruction(FVector_NetQuantize FieldPosition);

    void ApplyDestructionField(FVector FieldPosition);

    /** Override in Blueprint for VFX, sound, etc. */
    UFUNCTION(BlueprintNativeEvent)
    void OnDestructionTriggered(FVector HitLocation);
    virtual void OnDestructionTriggered_Implementation(FVector HitLocation) {}

private:
    void CheckAndTriggerDestruction();
    FVector GetDestructionFieldPosition() const;

    FVector LastDamageImpactPoint;
    bool bDestructionApplied = false;
};
