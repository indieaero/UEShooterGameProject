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

    /** Authoritative position selected on server and replicated for deterministic client destruction. */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Destruction")
    FVector_NetQuantize DestructionFieldPosition = FVector::ZeroVector;

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

    /** Apply additional physical impulse at impact point for clearer projectile-like reaction. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction")
    bool bUseImpactImpulse = true;

    /** Impulse strength for first break event. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0"))
    float BreakImpactImpulseStrength = 1200.0f;

    /** Impulse strength for post-break extra hits. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction", meta = (ClampMin = "0.0"))
    float PostBreakImpactImpulseStrength = 600.0f;

    /** Server batches post-break hits into one multicast per time window. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Destruction|Network", meta = (ClampMin = "0.01", ClampMax = "0.5"))
    float PostBreakBatchWindow = 0.06f;

    UFUNCTION()
    void OnRep_Health();

    UFUNCTION()
    void OnRep_IsDestroyed();

    void ApplyDestructionField(FVector FieldPosition);
    void ApplyPostBreakStrain(FVector FieldPosition, float MagnitudeScale);
    void ApplyImpactImpulse(FVector ImpactPoint, float ImpulseStrength);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPostBreakStrain(FVector_NetQuantize FieldPosition, float MagnitudeScale);

    /** Override in Blueprint for VFX, sound, etc. */
    UFUNCTION(BlueprintNativeEvent)
    void OnDestructionTriggered(FVector HitLocation);
    virtual void OnDestructionTriggered_Implementation(FVector HitLocation) {}

private:
    void CheckAndTriggerDestruction();
    FVector GetDestructionFieldPosition() const;
    void QueuePostBreakStrain(FVector ImpactPosition, float MagnitudeScale);
    void FlushQueuedPostBreakStrain();

    FVector LastDamageImpactPoint;
    bool bDestructionApplied = false;
    FTimerHandle PostBreakBatchTimerHandle;
    FVector PendingPostBreakImpactSum = FVector::ZeroVector;
    int32 PendingPostBreakHitCount = 0;
    float PendingPostBreakMagnitudeScale = 0.0f;
};
