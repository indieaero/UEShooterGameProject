// Shoot Them Up Game, All Rights Reserved.

#include "UI/STUKillfeedWidget.h"
#include "UI/STUKillfeedRowWidget.h"
#include "Components/VerticalBox.h"
#include "STUGameStateBase.h"

void USTUKillfeedWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorld* World = GetWorld())
    {
        if (ASTUGameStateBase* GameState = World->GetGameState<ASTUGameStateBase>())
        {
            KillFeedUpdatedHandle = GameState->OnKillFeedUpdated.AddUObject(this, &USTUKillfeedWidget::OnKillFeedUpdated);
        }
    }
}

void USTUKillfeedWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        if (ASTUGameStateBase* GameState = World->GetGameState<ASTUGameStateBase>())
        {
            GameState->OnKillFeedUpdated.Remove(KillFeedUpdatedHandle);
        }
    }
    Super::NativeDestruct();
}

void USTUKillfeedWidget::OnKillFeedUpdated(const FString& KillerName, const FString& VictimName)
{
    AddKillfeedEntry(FText::FromString(KillerName), FText::FromString(VictimName));
}

void USTUKillfeedWidget::AddKillfeedEntry(const FText& KillerName, const FText& VictimName, UTexture2D* WeaponIcon)
{
    if (!KillfeedRowWidgetClass || !KillfeedBox) return;

    const auto KillfeedRowWidget = CreateWidget<USTUKillfeedRowWidget>(GetWorld(), KillfeedRowWidgetClass);

    if (!KillfeedRowWidget) return;

    KillfeedRowWidget->SetKillerName(KillerName);
    KillfeedRowWidget->SetVictimName(VictimName);
    KillfeedRowWidget->SetWeaponIcon(WeaponIcon);

    KillfeedBox->AddChild(KillfeedRowWidget);

    while (KillfeedBox->GetChildrenCount() > MaxVisibleKillfeedRows)
    {
        if (UWidget* OldestRow = KillfeedBox->GetChildAt(0))
        {
            KillfeedBox->RemoveChild(OldestRow);
        }
        else
        {
            break;
        }
    }

    // TODO: Timer for 5–7 seconds to auto-remove the entry (RemoveChild + possible fade-out)
}

