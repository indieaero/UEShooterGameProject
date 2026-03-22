// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "STUKillfeedWidget.generated.h"

class UVerticalBox;
class ASTUGameStateBase;

UCLASS()
class SHOOTTHEMUP_API USTUKillfeedWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* KillfeedBox;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> KillfeedRowWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI", meta = (ClampMin = "1"))
    int32 MaxVisibleKillfeedRows = 6;

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

public:
    void AddKillfeedEntry(const FText& KillerName, const FText& VictimName, UTexture2D* WeaponIcon = nullptr);

private:
    void OnKillFeedUpdated(const FString& KillerName, const FString& VictimName);

    FDelegateHandle KillFeedUpdatedHandle;
};

