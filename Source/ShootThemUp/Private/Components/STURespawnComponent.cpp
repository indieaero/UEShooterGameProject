// Shoot Them Up Game, All Rights Reserved.

#include "Components/STURespawnComponent.h"
#include "Player/STUPlayerController.h"
#include "STUGameModeBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

USTURespawnComponent::USTURespawnComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USTURespawnComponent::Respawn(int32 RespawnTime)
{
    if (!GetWorld()) return;

    RespawnCountDown = RespawnTime;
    GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &USTURespawnComponent::RespawnTimerUpdate, 1.0f, true);

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        const bool bIsListenServerHost =
            (GetWorld()->GetNetMode() == NM_ListenServer) && (GetOwner() == UGameplayStatics::GetPlayerController(GetWorld(), 0));

        if (!bIsListenServerHost)
        {
            if (auto* PC = Cast<ASTUPlayerController>(GetOwner()))
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
    if(--RespawnCountDown == 0) 
    {
        if (!GetWorld()) return;
        GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);

        const auto GameMode = Cast<ASTUGameModeBase>(GetWorld()->GetAuthGameMode());
        if (!GameMode) return;

        GameMode->RespawnRequest(Cast<AController>(GetOwner()));
    }
}

