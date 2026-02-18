// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "STUBaseCharacter.generated.h"

class USTUHealthComponent;
class USTUWeaponComponent;
class USoundCue;

UCLASS()
class SHOOTTHEMUP_API ASTUBaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    ASTUBaseCharacter(const FObjectInitializer& ObjInit);

protected:
    //Create Health property for BaseCharacter using pointer to USTUHealthComponent that's we created
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USTUHealthComponent* HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USTUWeaponComponent* WeaponComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* DeathAnimMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float LifeSpanOnDeath = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    FVector2D LandedDamageVelocity = FVector2D(900.0f, 1200.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    FVector2D LandedDamage = FVector2D(10.0f, 100.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Material")
    FName MaterialColorName = "Paint Color";

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
    USoundCue* DeathSound;

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;
    virtual void OnDeath();
    virtual void OnHealthChanged(float Health, float HealthDelta);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    virtual void TurnOff() override;
    virtual void Reset() override;

    UFUNCTION(BlueprintCallable, Category= "Movement")
    virtual bool IsRunning() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetMovementDirection() const;

    void SetPlayerColor(const FLinearColor& Color);

    /** Aim rotation; for remote pawns uses replicated value so weapon pose is correct in multiplayer. */
    virtual FRotator GetBaseAimRotation() const override;

    /** For ABP: aim delta from character forward. (0,0,0) = looking forward; feed this to Aim Offset so (0,0) = center pose. */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    FRotator GetAimRotationRelativeToCharacter() const;

    UFUNCTION(Server, Unreliable)
    void ServerUpdateViewRotation(FRotator NewRotation);

    //getter for Controller 
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    USTUWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(ReplicatedUsing = OnRep_TeamColor)
    FLinearColor TeamColor = FLinearColor::White;

    /** Replicated view/aim rotation so remote clients can display correct weapon pose. */
    UPROPERTY(Replicated)
    FRotator ReplicatedViewRotation = FRotator::ZeroRotator;

    /** Set after first ServerUpdateViewRotation; until then GetBaseAimRotation returns horizontal for remote pawns. */
    UPROPERTY(Replicated)
    bool bReplicatedViewRotationSet = false;

    UFUNCTION()
    void OnRep_TeamColor();

private:
    UFUNCTION()
    void OnGroundLanded(const FHitResult& Hit);
};
