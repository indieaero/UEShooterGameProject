// Shoot Them Up Game, All Rights Reserved.

#include "STUGameInstance.h"
#include "STULocalPlayerPendingDisplayNameSubsystem.h"
#include "Sound/STUSoundFuncLib.h"
#include "HAL/IConsoleManager.h"
#include "Player/STUPlayerController.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

namespace STUPlayerNameConsole
{
    static constexpr int32 MaxDisplayNameLen = 48;

    static void SanitizeDisplayName(FString& InOut)
    {
        InOut.TrimStartAndEndInline();
        InOut.ReplaceInline(TEXT("\r"), TEXT(""), ESearchCase::CaseSensitive);
        InOut.ReplaceInline(TEXT("\n"), TEXT(" "), ESearchCase::CaseSensitive);
        if (InOut.Len() > MaxDisplayNameLen)
        {
            InOut.LeftInline(MaxDisplayNameLen);
        }
    }
}  // namespace STUPlayerNameConsole

static FAutoConsoleCommandWithWorldAndArgs GSetPlayerNameCmd(
    TEXT("SetPlayerName"),
    TEXT("Sets your display name for this session (killfeed, game over). Usage: SetPlayerName Your Nickname"),
    FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&USTUGameInstance::ConsoleSetPlayerName),
    ECVF_Default);

void USTUGameInstance::ConsoleSetPlayerName(const TArray<FString>& Args, UWorld* World)
{
    if (Args.Num() == 0 || !World)
    {
        return;
    }

    USTUGameInstance* GI = Cast<USTUGameInstance>(World->GetGameInstance());
    if (!GI)
    {
        return;
    }

    const FString Name = FString::Join(Args, TEXT(" "));
    GI->SetPendingPlayerDisplayName(Name);
    if (GI->GetPendingPlayerDisplayName().IsEmpty())
    {
        return;
    }

    // PIE / listen: one GameInstance can back multiple LocalPlayers — scope pending to the viewport that issued the command.
    if (UGameViewportClient* WorldViewport = World->GetGameViewport())
    {
        for (ULocalPlayer* LP : GI->GetLocalPlayers())
        {
            if (!LP)
            {
                continue;
            }
            if (LP->ViewportClient != WorldViewport)
            {
                continue;
            }
            if (USTULocalPlayerPendingDisplayNameSubsystem* Sub = LP->GetSubsystem<USTULocalPlayerPendingDisplayNameSubsystem>())
            {
                Sub->SetPendingDisplayName(GI->GetPendingPlayerDisplayName());
            }
        }
    }

    GI->TryApplyPendingDisplayName(World);
}

void USTUGameInstance::SetPendingPlayerDisplayName(const FString& Name)
{
    FString Copy = Name;
    STUPlayerNameConsole::SanitizeDisplayName(Copy);
    PendingPlayerDisplayName = MoveTemp(Copy);
}

void USTUGameInstance::TryApplyPendingDisplayName(UWorld* World)
{
    if (!World)
    {
        return;
    }

    // Listen server / PIE: more than one local PlayerController in the same world; GetFirstPlayerController() is only player 0.
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        if (ASTUPlayerController* STUPC = Cast<ASTUPlayerController>(It->Get()))
        {
            if (STUPC->IsLocalController())
            {
                STUPC->ApplyPendingDisplayNameFromSettings();
            }
        }
    }
}

void USTUGameInstance::ToggleVolume()
{
    USTUSoundFuncLib::ToggleSoundClassVolume(MasterSoundClass);
}
