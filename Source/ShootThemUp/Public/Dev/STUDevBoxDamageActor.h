// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STUDevBoxDamageActor.generated.h"

UCLASS()
class SHOOTTHEMUP_API ASTUDevBoxDamageActor : public AActor
{
	GENERATED_BODY()

public:
	ASTUDevBoxDamageActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector HalfExtents = FVector(300.f, 300.f, 100.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FColor BoxColor = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UDamageType> DamageType;

	/** Server only: draw debug box. Does not affect damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = true;

	virtual void Tick(float DeltaTime) override;
};
