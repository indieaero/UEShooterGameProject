// Shoot Them Up Game, All Rights Reserved.

#include "AI/STUAIController.h"
#include "AI/STUAICharacter.h"

void ASTUAIController::OnPossess(APawn* InPawn) 
{
    Super::OnPossess(InPawn);

    const auto STUCharacter = Cast<ASTUAICharacter>(InPawn);
    if (STUCharacter)
    {
        RunBehaviorTree(STUCharacter->BehaviorTreeAsset);
    }
}
