// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/STUBaseWeapon.h"
#include "STULauncherWeapon.generated.h"

class ASTUProjectile;
class USoundCue;
class UCameraShakeBase;

UCLASS()
class SHOOTTHEMUP_API ASTULauncherWeapon : public ASTUBaseWeapon
{
	GENERATED_BODY()

public:
	virtual void StartFire() override;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFireFX();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
    TSubclassOf<ASTUProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
    USoundCue* NoAmmoSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
    float RecoilPitchPerShot = 0.35f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
    float RecoilYawRandom = 0.15f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX");
    TSubclassOf<UCameraShakeBase> CameraShake;

    virtual void MakeShot() override;

private:
    void PlayLauncherCameraShake();
};
