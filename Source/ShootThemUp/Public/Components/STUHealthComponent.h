// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "STUHealthComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTTHEMUP_API USTUHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USTUHealthComponent();

      //Create a function for the Health variable that will return its value
      float GetHealth() const { return Health; }


protected:

	//Create max health variable and can change it in Blueprints
      UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
      float MaxHealth = 100.0f;

	// Called when the game starts
	virtual void BeginPlay() override;

private:
      float Health = 0.0f;
};
