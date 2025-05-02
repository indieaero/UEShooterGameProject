// Shoot Them Up Game, All Rights Reserved.

#include "AI/Services/STUFireService.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/STUWeaponComponent.h"
#include "STUUtils.h"


USTUFireService::USTUFireService() 
{
    NodeName = "Fire";
}

void USTUFireService::TickNode(UBehaviorTreeComponent& OnwnerComp, uint8* NodeMemory, float DeltaSeconds) 
{
    const auto Controller = OnwnerComp.GetAIOwner();
    const auto Blackboard = OnwnerComp.GetBlackboardComponent();

    const auto HasAim = Blackboard && Blackboard->GetValueAsObject(EnemyActorKey.SelectedKeyName);

    if (Controller)
    {
        const auto WeaponComponent = STUUtils::GetSTUPlayerComponent<USTUWeaponComponent>(Controller->GetPawn());
        if (WeaponComponent)
        {
            HasAim ? WeaponComponent->StartFire() : WeaponComponent->StartFire();
        }
    }

    Super::TickNode(OnwnerComp, NodeMemory, DeltaSeconds);
}


