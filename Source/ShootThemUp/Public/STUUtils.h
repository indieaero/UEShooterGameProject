#pragma once
#include "Player/STUPlayerState.h"
#include "GameFramework/Controller.h"

class STUUtils
{
public:
    template <typename T>
    static T* GetSTUPlayerComponent(AActor* Actor)
    {
        if (!Actor) return nullptr;
        const auto Component = Actor->GetComponentByClass(T::StaticClass());
        return Cast<T>(Component);
    }

    // Overload for controller (e.g. RespawnComponent on PlayerController); AController inherits AActor
    template <typename T>
    static T* GetSTUPlayerComponent(AController* Controller)
    {
        if (!Controller) return nullptr;
        return GetSTUPlayerComponent<T>(static_cast<AActor*>(Controller));
    }

    bool static AreEnemy(AController* Controller1, AController* Controller2)
    {
        if (!Controller1 || !Controller2 || Controller1 == Controller2) return false;

        const auto PlayerState1 = Cast<ASTUPlayerState>(Controller1->PlayerState);
        const auto PlayerState2 = Cast<ASTUPlayerState>(Controller2->PlayerState);

        return PlayerState1 && PlayerState2 && PlayerState1->GetTeamID() != PlayerState2->GetTeamID();
    }

    static FText TextFromInt(int32 Number) { return FText::FromString(FString::FromInt(Number)); }
};