// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "STULocalPlayerPendingDisplayNameSubsystem.generated.h"

/** Per-local-player pending nickname (PIE / split-screen can share one USTUGameInstance). */
UCLASS()
class SHOOTTHEMUP_API USTULocalPlayerPendingDisplayNameSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    void SetPendingDisplayName(const FString& Name);
    const FString& GetPendingDisplayName() const { return PendingDisplayName; }

private:
    FString PendingDisplayName;
};
