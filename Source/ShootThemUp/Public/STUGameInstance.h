// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "STUCoreTypes.h"
#include "STUGameInstance.generated.h"

class USoundClass;

UCLASS()
class SHOOTTHEMUP_API USTUGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    /** Console: SetPlayerName <nickname> — registered globally (see .cpp). */
    static void ConsoleSetPlayerName(const TArray<FString>& Args, UWorld* World);

    FLevelData GetStartUpLevel() const { return StartUpLevel; }
    void SetStartUpLevel(const FLevelData& LevelData) { StartUpLevel = LevelData; }

    TArray<FLevelData> GetLevelsData() const { return LevelsData; }

    FName GetMenuLevelName() const { return MenuLevelName; }

    void ToggleVolume();

    /** Nickname used when connecting to a server (set via SetPlayerName console command). */
    const FString& GetPendingPlayerDisplayName() const { return PendingPlayerDisplayName; }

    void SetPendingPlayerDisplayName(const FString& Name);

    /** If the local player uses ASTUPlayerController, push the pending name to the server. */
    void TryApplyPendingDisplayName(UWorld* World);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Game", meta = (ToolTip = "Level names must me unique!"))
    TArray<FLevelData> LevelsData;

    UPROPERTY(EditDefaultsOnly, Category = "Game")
    FName MenuLevelName = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    USoundClass* MasterSoundClass;

private:
    FLevelData StartUpLevel;

    FString PendingPlayerDisplayName;
};
