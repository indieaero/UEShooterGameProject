// Shoot Them Up Game, All Rights Reserved.

#include "Components/STUStaminaComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

USTUStaminaComponent::USTUStaminaComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void USTUStaminaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USTUStaminaComponent, CurrentStamina);
    DOREPLIFETIME(USTUStaminaComponent, MaxStamina);
    DOREPLIFETIME(USTUStaminaComponent, bIsRecoveryDelayed);
}

void USTUStaminaComponent::BeginPlay()
{
    Super::BeginPlay();

    check(MaxStamina > 0.0f);

    AActor* Owner = GetOwner();
    if (Owner && Owner->HasAuthority())
    {
        CurrentStamina = MaxStamina;
        bIsRecoveryDelayed = false;
    }
}

bool USTUStaminaComponent::CanRun() const
{
    return CurrentStamina > 0.0f && !bIsRecoveryDelayed;
}

bool USTUStaminaComponent::ShouldShowStaminaBar() const
{
    return CurrentStamina < MaxStamina;
}

void USTUStaminaComponent::UpdateStamina(float DeltaTime, bool bWantsToRunAndMoving)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Owner->HasAuthority()) return;

    if (bWantsToRunAndMoving && CanRun())
    {
        DrainStamina(DeltaTime);
    }
    else
    {
        UpdateRecovery(DeltaTime);
    }
}

void USTUStaminaComponent::DrainStamina(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (World) World->GetTimerManager().ClearTimer(RecoveryDelayHandle);

    const float DrainAmount = StaminaDrainRate * DeltaTime;
    const float NewStamina = FMath::Max(0.0f, CurrentStamina - DrainAmount);

    if (NewStamina <= 0.0f)
    {
        SetCurrentStamina(0.0f);
        bIsRecoveryDelayed = true;
        OnStaminaDepleted.Broadcast();

        if (World)
        {
            World->GetTimerManager().SetTimer(RecoveryDelayHandle, this, &USTUStaminaComponent::OnRecoveryDelayComplete,
                StaminaRecoveryDelay, false);
        }
    }
    else
    {
        SetCurrentStamina(NewStamina);
    }
}

void USTUStaminaComponent::UpdateRecovery(float DeltaTime)
{
    if (bIsRecoveryDelayed || CurrentStamina >= MaxStamina) return;

    const float RecoverAmount = StaminaRecoveryRate * DeltaTime;
    SetCurrentStamina(FMath::Min(MaxStamina, CurrentStamina + RecoverAmount));
}

void USTUStaminaComponent::SetCurrentStamina(float NewStamina)
{
    if (!GetOwner() || !GetOwner()->HasAuthority()) return;
    CurrentStamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
}

void USTUStaminaComponent::OnRep_CurrentStamina()
{
    // Optional: broadcast for UI updates if needed
}

void USTUStaminaComponent::OnRecoveryDelayComplete()
{
    bIsRecoveryDelayed = false;
}
