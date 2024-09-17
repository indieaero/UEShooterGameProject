// Shoot Them Up Game, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "STUGameHUD.generated.h"

/**
 *
 */
UCLASS()
class SHOOTTHEMUP_API ASTUGameHUD : public AHUD
{
    GENERATED_BODY()

public:
    //virtual function from AHUD class that can draw lines, text, etc.
    virtual void DrawHUD() override;

private:
    //create separate function where put logic of drawing crosshair
    void DrawCrossHair();
};
