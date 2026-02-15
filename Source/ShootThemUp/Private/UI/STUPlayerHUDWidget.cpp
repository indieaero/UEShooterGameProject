// Shoot Them Up Game, All Rights Reserved.

#include "UI/STUPlayerHUDWidget.h"
#include "Components/STUHealthComponent.h"
#include "Components/STUWeaponComponent.h"
#include "Components/STURespawnComponent.h"
#include "GameFramework/Pawn.h"
#include "STUUtils.h"
#include "Components/ProgressBar.h"
#include "Player/STUPlayerState.h"
#include "TimerManager.h"

float USTUPlayerHUDWidget::GetHealthPercent() const
{
    const auto HealthComponent = STUUtils::GetSTUPlayerComponent<USTUHealthComponent>(GetOwningPlayerPawn());
    if (!HealthComponent) return 0.0f;

    return HealthComponent->GetHealthPercent();
}

bool USTUPlayerHUDWidget::GetCurrentWeaponUIData(FWeaponUIData& UIData) const
{
    const auto WeaponComponent = STUUtils::GetSTUPlayerComponent<USTUWeaponComponent>(GetOwningPlayerPawn());
    if (!WeaponComponent) return false;

    return WeaponComponent->GetCurrentWeaponUIData(UIData);
}

bool USTUPlayerHUDWidget::GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const
{
    const auto WeaponComponent = STUUtils::GetSTUPlayerComponent<USTUWeaponComponent>(GetOwningPlayerPawn());
    if (!WeaponComponent) return false;

    return WeaponComponent->GetCurrentWeaponAmmoData(AmmoData);
}

bool USTUPlayerHUDWidget::IsPlayerAlive() const
{
    const auto HealthComponent = STUUtils::GetSTUPlayerComponent<USTUHealthComponent>(GetOwningPlayerPawn());
    return HealthComponent && !HealthComponent->IsDead();
}

bool USTUPlayerHUDWidget::IsPlayerSpectating() const
{
    const auto Controller = GetOwningPlayer();
    if (!Controller) return false;

    if (Controller->GetStateName() == NAME_Spectating) return true;
    return IsRespawnInProgress();
}

bool USTUPlayerHUDWidget::IsRespawnInProgress() const
{
    const auto RespawnComponent = STUUtils::GetSTUPlayerComponent<USTURespawnComponent>(GetOwningPlayer());
    return RespawnComponent && RespawnComponent->IsRespawnInProgress();
}

void USTUPlayerHUDWidget::NativeDestruct()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HealthBarRefreshHandle);
    }
    Super::NativeDestruct();
}

void USTUPlayerHUDWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(HealthBarRefreshHandle, this,
            &USTUPlayerHUDWidget::UpdateHealthBar, 0.1f, true);
    }

    if (GetOwningPlayer())
    {
        GetOwningPlayer()->GetOnNewPawnNotifier().AddUObject(this, &USTUPlayerHUDWidget::OnNewPawn);
        OnNewPawn(GetOwningPlayerPawn());
    }
}

void USTUPlayerHUDWidget::OnNewPawn(APawn*)
{
    LastKnownHealth = -1.0f;
}

void USTUPlayerHUDWidget::UpdateHealthBar()
{
    const float Percent = GetHealthPercent();

    if (HealthProgressBar)
    {
        HealthProgressBar->SetPercent(Percent);
        HealthProgressBar->SetFillColorAndOpacity(Percent > PercentColorThreshold ? GoodColor : BadColor);
    }

    if (LastKnownHealth >= 0.0f && Percent < LastKnownHealth - KINDA_SMALL_NUMBER)
    {
        OnTakeDamage();
    }
    LastKnownHealth = Percent;
}

int32 USTUPlayerHUDWidget::GetKillsNum() const
{
    const auto Controller = GetOwningPlayer();
    if (!Controller) return 0;

    const auto PlayerState = Cast<ASTUPlayerState>(Controller->PlayerState);
    return PlayerState ? PlayerState->GetKillsNum() : 0;
}

FString USTUPlayerHUDWidget::FormatBullets(int32 BulletsNum) const
{
    const int32 MaxLen = 3;
    const TCHAR PrefixSymbol = '0';

    auto BulletsStr = FString::FromInt(BulletsNum);
    const auto SymbolsNumToAdd = MaxLen - BulletsStr.Len();

    if(SymbolsNumToAdd > 0)
    {
        BulletsStr = FString::ChrN(SymbolsNumToAdd, PrefixSymbol).Append(BulletsStr);
    }

    return BulletsStr;
}

