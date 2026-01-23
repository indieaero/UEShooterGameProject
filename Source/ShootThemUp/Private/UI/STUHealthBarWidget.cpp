// Shoot Them Up Game, All Rights Reserved.

#include "UI/STUHealthBarWidget.h"
#include "Components/ProgressBar.h"

void USTUHealthBarWidget::SetHealthPercent(float Percent)
{
    if (!HealthProgressBar) return;

    const auto HealthbarVisibility = Percent > PercentVisibilityThreshold || FMath::IsNearlyZero(Percent)  //
                                         ? ESlateVisibility::Hidden
                                         : ESlateVisibility::Visible;
    HealthProgressBar->SetVisibility(HealthbarVisibility);

    const auto HealthBarColor = Percent > PercentColorThreshold ? GoodColor : BadColor;
    HealthProgressBar->SetFillColorAndOpacity(HealthBarColor);

    HealthProgressBar->SetPercent(Percent);
}
