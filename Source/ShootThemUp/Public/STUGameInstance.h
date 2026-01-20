// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "STUCoreTypes.h"
#include "STUGameInstance.generated.h"

UCLASS()
class SHOOTTHEMUP_API USTUGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    FLevelData GetStartUpLevel() const { return StartUpLevel; }
    void SetStartUpLevel(const FLevelData& LevelData) { StartUpLevel = LevelData; }

    TArray<FLevelData> GetLevelsData() const { return LevelsData; }

    FName GetMenuLevelName() const { return MenuLevelName; }

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Game", meta = (ToolTip = "Level names must me unique!"))
    TArray<FLevelData> LevelsData;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    FName MenuLevelName = NAME_None;

private:
    FLevelData StartUpLevel;
};
