// Shoot Them Up Game, All Rights Reserved.

#include "UI/STUKillfeedRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void USTUKillfeedRowWidget::SetKillerName(const FText& KillerName)
{
    if (!KillerNameTextBlock) return;
    KillerNameTextBlock->SetText(KillerName);
}

void USTUKillfeedRowWidget::SetVictimName(const FText& VictimName)
{
    if (!VictimNameTextBlock) return;
    VictimNameTextBlock->SetText(VictimName);
}

void USTUKillfeedRowWidget::SetWeaponIcon(UTexture2D* Icon)
{
    if (!WeaponIconImage) return;
    if (Icon)
    {
        WeaponIconImage->SetBrushFromTexture(Icon);
        WeaponIconImage->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        WeaponIconImage->SetVisibility(ESlateVisibility::Visible);  // Показать дефолтный Brush из Blueprint (пуля/placeholder)
    }
}
