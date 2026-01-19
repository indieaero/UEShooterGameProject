// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "STUGameInstance.generated.h"

UCLASS()
class SHOOTTHEMUP_API USTUGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    FName GetStartUpLevelName() const { return StartUpLevelName; }
    FName GetMenuLevelName() const { return MenuLevelName; }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    FName StartUpLevelName = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    FName MenuLevelName = NAME_None;
};
