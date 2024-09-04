// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "STUDevDamageActor.generated.h"

UCLASS()
class SHOOTTHEMUP_API ASTUDevDamageActor : public AActor
{
    GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
    ASTUDevDamageActor();

	//Add a SceneComponent to Actor for it has a transformation, and we can move it around the scene
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    USceneComponent* SceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FColor SphereColor = FColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool DoFullDamage = false;

    //Create Property that pass the class of which we want to deal damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UDamageType> DamageType;

protected:
	// Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:	
	// Called every frame
    virtual void Tick(float DeltaTime) override;

};
