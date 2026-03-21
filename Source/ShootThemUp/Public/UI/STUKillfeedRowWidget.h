// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "STUKillfeedRowWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class SHOOTTHEMUP_API USTUKillfeedRowWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetKillerName(const FText& KillerName);
    void SetVictimName(const FText& VictimName);
    void SetWeaponIcon(UTexture2D* Icon);

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KillerNameTextBlock;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VictimNameTextBlock;

    UPROPERTY(meta = (BindWidget))
    class UImage* WeaponIconImage;
};
