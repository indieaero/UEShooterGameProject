// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/STUBaseCharacter.h"
#include "STUPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USphereComponent;
class USTUStaminaComponent;

UCLASS()
class SHOOTTHEMUP_API ASTUPlayerCharacter : public ASTUBaseCharacter
{
    GENERATED_BODY()

public:
    ASTUPlayerCharacter(const FObjectInitializer& ObjInit);

    // Server functions
    UFUNCTION(Server, Reliable)
    void ServerSetRunning(bool bNewRunning);

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
};
