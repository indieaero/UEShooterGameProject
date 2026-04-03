// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/STUBaseCharacter.h"
#include "STUPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USphereComponent;
class USTUStaminaComponent;
class UMaterialInterface;
class UAudioComponent;
class UAnimMontage;
class USkeletalMeshComponent;

UCLASS()
class SHOOTTHEMUP_API ASTUPlayerCharacter : public ASTUBaseCharacter
{
    GENERATED_BODY()

public:
    ASTUPlayerCharacter(const FObjectInitializer& ObjInit);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool CanKick() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TryKick();

    // Server functions
    UFUNCTION(Server, Reliable)
    void ServerSetRunning(bool bNewRunning);

    UFUNCTION(Server, Reliable)
    void ServerTryKick();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayKickMontage();

    UFUNCTION(BlueprintImplementableEvent, Category = "Zoom")
    void OnZoomStateChanged(bool bIsZooming);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USpringArmComponent* SpringArmComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USphereComponent* CameraCollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    USTUStaminaComponent* StaminaComponent;

    // Post-process material (e.g. M_RunBlur_Inst) applied to camera when running 
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
    UMaterialInterface* RunBlurMaterial;

    // Post-process material (e.g. M_PlayerDeath) applied to camera when player dies 
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX")
    UMaterialInterface* DeathPostProcessMaterial;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* KickMontage = nullptr;

    // Max horizontal speed (cm/s) on ground to count as "standing" for kick. 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation", meta = (ClampMin = "0.0"))
    float KickIdleHorizontalSpeedThreshold = 15.0f;

    virtual void OnDeath() override;
    virtual void BeginPlay() override;

    // Register replication of properties (WantsToRun,IsMovingForward)
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

    virtual bool IsRunning() const override;

    USTUStaminaComponent* GetStaminaComponent() const { return StaminaComponent; }

private:
    UPROPERTY(Replicated)
    bool WantsToRun = false;

    bool IsMovingForward = false;

    void MoveForward(float Amount);
    void MoveRight(float Amount);
    void CheckAndJump();
    void OnKickPressed();

    void InitKickNotify();
    void OnKickMontageFinished(USkeletalMeshComponent* MeshComp);
    void StartKickMontage();

    bool KickAnimInProgress = false;

    void OnStartRunning();
    void OnStopRunning();
    void OnStaminaDepleted();

    UFUNCTION()
    void OnCameraCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    void OnCameraCollisionEndOverlap(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void CheckCameraOverlap();

    void UpdateRunningSounds();

    UPROPERTY()
    UAudioComponent* RunningVoiceComponent = nullptr;

    UPROPERTY()
    UAudioComponent* TiredVoiceComponent = nullptr;
};
