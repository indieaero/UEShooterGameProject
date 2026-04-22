// Shoot Them Up Game, All Rights Reserved.

#include "Components/STURespawnComponent.h"
#include "Player/STUPlayerController.h"
#include "STUGameModeBase.h"
#include "Engine/World.h"

USTURespawnComponent::USTURespawnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USTURespawnComponent::Respawn(int32 RespawnTime)
{
    if (!GetWorld()) return;

    RespawnCountDown = RespawnTime;
    GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &USTURespawnComponent::RespawnTimerUpdate, 1.0f, true);

    // Server notifies only remote owning client to run countdown locally.
    // Standalone and listen-server host are local+authority and should not receive this RPC.
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        if (auto* PC = Cast<ASTUPlayerController>(GetOwner()))
        {
            if (!PC->IsLocalController())
            {
                PC->ClientStartRespawnTimer(RespawnTime);
            }
        }
    }
}

bool USTURespawnComponent::IsRespawnInProgress() const
{
    return GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(RespawnTimerHandle);
}

void USTURespawnComponent::RespawnTimerUpdate()
{
    if (--RespawnCountDown == 0)
    {
        if (!GetWorld()) return;
        GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);

        // Only server can request respawn (GameMode is server-only)
        if (!GetOwner() || !GetOwner()->HasAuthority()) return;
        const auto GameMode = Cast<ASTUGameModeBase>(GetWorld()->GetAuthGameMode());
        if (!GameMode) return;

        GameMode->RespawnRequest(Cast<AController>(GetOwner()));
    }
}

