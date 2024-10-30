// Shoot Them Up Game, All Rights Reserved.

#include "UI/STUPlayerHUDWidget.h"
#include "Components/STUHealthComponent.h"
#include "Components/STUWeaponComponent.h"
#include "GameFramework/Pawn.h"

float USTUPlayerHUDWidget::GetHealthPercent() const
{
    //variable Player is created, which stores a reference to the game character that owns this HUD widget
    const auto Player = GetOwningPlayerPawn();
    if (!Player) return 0.0f;

    //Using GetComponentByClass attempts to get the USTUHealthComponent component from the player class (Pawn). A pointer to the component is returned if it exists.
    const auto Component = Player->GetComponentByClass(USTUHealthComponent::StaticClass());
    const auto HealthComponent = Cast<USTUHealthComponent>(Component);
    if (!HealthComponent) return 0.0f;

    return HealthComponent->GetHealthPercent();

}

bool USTUPlayerHUDWidget::GetCurrentWeaponUIData(FWeaponUIData& UIData) const
{
    const auto WeaponComponent = GetWeaponComponent();
    if (!WeaponComponent) return false;

    return WeaponComponent->GetCurrentWeaponUIData(UIData);
}

bool USTUPlayerHUDWidget::GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const
{
    const auto WeaponComponent = GetWeaponComponent();
    if (!WeaponComponent) return false;

    return WeaponComponent->GetCurrentWeaponAmmoData(AmmoData);
}

USTUWeaponComponent* USTUPlayerHUDWidget::GetWeaponComponent() const
{
    const auto Player = GetOwningPlayerPawn();
    if (!Player) return nullptr;

    const auto Component = Player->GetComponentByClass(USTUWeaponComponent::StaticClass());
    const auto WeaponComponent = Cast<USTUWeaponComponent>(Component);
    return WeaponComponent;
}