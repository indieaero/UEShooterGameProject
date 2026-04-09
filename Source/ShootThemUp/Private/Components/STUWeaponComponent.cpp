// Shoot Them Up Game, All Rights Reserved.

#include "Components/STUWeaponComponent.h"
#include "Weapon/STUBaseWeapon.h"
#include "GameFramework/Character.h"
#include "STUBaseCharacter.h"
#include "Animations/STUEquipFinishedAnimNotify.h"
#include "Animations/STUReloadFinishedAnimNotify.h"
#include "Animations/AnimUtils.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "Player/STUPlayerCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(logWeaponComponent, All, All)

constexpr static int32 WeaponNum = 2;

USTUWeaponComponent::USTUWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void USTUWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(USTUWeaponComponent, CurrentWeaponIndex);  // Clients sync equipped weapon
}

void USTUWeaponComponent::BeginPlay()
{
    Super::BeginPlay();

    checkf(WeaponData.Num() == WeaponNum, TEXT("Our character can hold only %i weapon items"), WeaponNum);

    InitAnimations();
    CurrentWeaponIndex = 0;

    // Server spawns weapons; client finds them from replicated attached actors
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        SpawnWeapons();
        EquipWeapon(CurrentWeaponIndex);
    }
    else
    {
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimerForNextTick(this, &USTUWeaponComponent::DiscoverReplicatedWeapons);
        }
    }
}

void USTUWeaponComponent::DiscoverReplicatedWeapons()
{
    // Client: collect weapons replicated from server (attached to character mesh)
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    TArray<AActor*> AttachedActors;
    Character->GetAttachedActors(AttachedActors, false, true);
    Weapons.Empty();
    for (AActor* Actor : AttachedActors)
    {
        if (ASTUBaseWeapon* Weapon = Cast<ASTUBaseWeapon>(Actor))
        {
            Weapons.Add(Weapon);
        }
    }
    // Sort to match WeaponData order (Rifle first, Launcher second, etc.)
    Weapons.Sort([this](const ASTUBaseWeapon& A, const ASTUBaseWeapon& B)
    {
        int32 IndexA = WeaponData.IndexOfByPredicate([&](const FWeaponData& D) { return D.WeaponClass == A.GetClass(); });
        int32 IndexB = WeaponData.IndexOfByPredicate([&](const FWeaponData& D) { return D.WeaponClass == B.GetClass(); });
        if (IndexA == INDEX_NONE) IndexA = 999;
        if (IndexB == INDEX_NONE) IndexB = 999;
        return IndexA < IndexB;
    });
    if (Weapons.Num() > 0 && CurrentWeaponIndex >= 0 && CurrentWeaponIndex < Weapons.Num())
    {
        CurrentWeapon = Weapons[CurrentWeaponIndex];
    }
    if (Weapons.Num() < WeaponNum && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &USTUWeaponComponent::DiscoverReplicatedWeapons);
    }
}

void USTUWeaponComponent::OnRep_CurrentWeaponIndex()
{
    // Sync current weapon when index replicates from server
    if (Weapons.IsValidIndex(CurrentWeaponIndex))
    {
        CurrentWeapon = Weapons[CurrentWeaponIndex];
    }
}

void USTUWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CurrentWeapon = nullptr;
    if (GetOwner() && GetOwner()->HasAuthority())  // Only server owns and destroys weapon actors
    {
        for (auto Weapon : Weapons)
        {
            if (Weapon)
            {
                Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
                Weapon->Destroy();
            }
        }
    }
    Weapons.Empty();

    Super::EndPlay(EndPlayReason);
}

void USTUWeaponComponent::SpawnWeapons()
{
    // Cast the owner of the component to ACharacter. If the cast fails (owner is not a character), return
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || !GetWorld()) return;

    for (auto OneWeaponData : WeaponData)
    {
        // Spawn WeaponClass using GetWorld
        auto Weapon = GetWorld()->SpawnActor<ASTUBaseWeapon>(OneWeaponData.WeaponClass);
        if (!Weapon) continue;

        Weapon->OnClipEmpty.AddUObject(this, &USTUWeaponComponent::OnEmptyClip);
        Weapon->SetOwner(Character);
        Weapons.Add(Weapon);

        AttachWeaponToSocket(Weapon, Character->GetMesh(), WeaponArmorySocketName);
    }
}

void USTUWeaponComponent::AttachWeaponToSocket(ASTUBaseWeapon* Weapon, USceneComponent* SceneComponent, const FName& SocketName)
{
    if (!Weapon || !SceneComponent) return;

    FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
    Weapon->AttachToComponent(SceneComponent, AttachmentRules, SocketName);
}

void USTUWeaponComponent::EquipWeapon(int32 WeaponIndex)
{
    if (WeaponIndex < 0 || WeaponIndex >= Weapons.Num())
    {
        UE_LOG(logWeaponComponent, Warning, TEXT("Invalid weapon index"));
        return;
    }
    if (GetOwner() && !GetOwner()->HasAuthority()) return;  // Server only: attachment replicates

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    if (CurrentWeapon)
    {
        CurrentWeapon->Zoom(false);
        CurrentWeapon->StopFire();
        AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponArmorySocketName);
    }

    CurrentWeapon = Weapons[WeaponIndex];
    //CurrentReloadAnimMontage = WeaponData[WeaponIndex].ReloadAnimMontage;
    const auto CurrentWeaponData = WeaponData.FindByPredicate([&](const FWeaponData& Data) {  //
        return Data.WeaponClass == CurrentWeapon->GetClass();                                 //
    });

    CurrentReloadAnimMontage = CurrentWeaponData ? CurrentWeaponData->ReloadAnimMontage : nullptr;

    AttachWeaponToSocket(CurrentWeapon, Character->GetMesh(), WeaponEquipSocketName);

    EquipAnimInProgress = true;

    PlayAnimMontage(EquipAnimMontage);
    MulticastPlayEquipAnim();
}

void USTUWeaponComponent::StartFire()
{
    if (GetOwner() && GetOwner()->GetLocalRole() < ROLE_Authority)  // Client: request server to fire
    {
        ServerStartFire();
        return;
    }
    if (!CanFire()) return;
    if (CurrentWeapon) CurrentWeapon->StartFire();
}

void USTUWeaponComponent::ServerStartFire_Implementation()
{
    if (!CanFire()) return;
    if (CurrentWeapon) CurrentWeapon->StartFire();  // Server runs actual fire logic
}

void USTUWeaponComponent::StopFire()
{
    if (GetOwner() && GetOwner()->GetLocalRole() < ROLE_Authority)  // Client: request server
    {
        ServerStopFire();
        return;
    }
    if (CurrentWeapon) CurrentWeapon->StopFire();
}

void USTUWeaponComponent::ServerStopFire_Implementation()
{
    if (CurrentWeapon) CurrentWeapon->StopFire();
}

void USTUWeaponComponent::NextWeapon()
{
    if (GetOwner() && GetOwner()->GetLocalRole() < ROLE_Authority)  // Client: request server to switch
    {
        ServerNextWeapon();
        return;
    }
    if (!CanEquip() || Weapons.Num() == 0) return;
    CurrentWeaponIndex = (CurrentWeaponIndex + 1) % Weapons.Num();
    EquipWeapon(CurrentWeaponIndex);
}

void USTUWeaponComponent::ServerNextWeapon_Implementation()
{
    if (!CanEquip() || Weapons.Num() == 0) return;
    CurrentWeaponIndex = (CurrentWeaponIndex + 1) % Weapons.Num();
    EquipWeapon(CurrentWeaponIndex);
}

void USTUWeaponComponent::ServerReload_Implementation()
{
    ChangeClip();  // Server runs reload; MulticastPlayReloadAnim notifies clients
}

void USTUWeaponComponent::PlayAnimMontage(UAnimMontage* Animation)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character) return;

    Character->PlayAnimMontage(Animation);
}

void USTUWeaponComponent::InitAnimations()
{
    auto EquipFinishedNotify = AnimUtils::FindNotifyByClass<USTUEquipFinishedAnimNotify>(EquipAnimMontage);
    if (EquipFinishedNotify)
    {
        EquipFinishedNotify->OnNotified.AddUObject(this, &USTUWeaponComponent::OnEquipFinished);
    }else
    {
        UE_LOG(logWeaponComponent, Error, TEXT("Equip anim notify is forgotten to set"));
        checkNoEntry();
    }

    for (auto OneWeaponData : WeaponData)
    {
        auto ReloadFinishedNotify = AnimUtils::FindNotifyByClass<USTUReloadFinishedAnimNotify>(OneWeaponData.ReloadAnimMontage);
        if (!ReloadFinishedNotify)
        {
            UE_LOG(logWeaponComponent, Error, TEXT("Reload anim notify is forgotten to set"));
            checkNoEntry();
        }

        ReloadFinishedNotify->OnNotified.AddUObject(this, &USTUWeaponComponent::OnReloadFinished);
    }
}

void USTUWeaponComponent::OnEquipFinished(USkeletalMeshComponent* MeshComp)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || MeshComp != Character->GetMesh()) return;

    EquipAnimInProgress = false;
}

void USTUWeaponComponent::OnReloadFinished(USkeletalMeshComponent* MeshComp)
{
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character || MeshComp != Character->GetMesh()) return;

    ReloadAnimInProgress = false;
}

bool USTUWeaponComponent::CanFire() const
{
    const auto Player = Cast<ASTUBaseCharacter>(GetOwner());
    if (!Player) return false;

    const auto PlayerCharacter = Cast<ASTUPlayerCharacter>(Player);
    const bool bKickInProgress = PlayerCharacter && PlayerCharacter->IsKickInProgress();

    return CurrentWeapon && !Player->IsRunning() && !EquipAnimInProgress && !ReloadAnimInProgress && !bKickInProgress;
}

bool USTUWeaponComponent::CanEquip() const
{
    return !EquipAnimInProgress && !ReloadAnimInProgress;
}

bool USTUWeaponComponent::CanReload() const
{
    return CurrentWeapon             //
           && !EquipAnimInProgress   //
           && !ReloadAnimInProgress  //
           && CurrentWeapon->CanReload();
}

void USTUWeaponComponent::Reload()
{
    if (GetOwner() && GetOwner()->GetLocalRole() < ROLE_Authority)  // Client: request server
    {
        ServerReload();
        return;
    }
    ChangeClip();
}

void USTUWeaponComponent::OnEmptyClip(ASTUBaseWeapon* AmmoEmptyWeapon)
{
    if (!AmmoEmptyWeapon) return;

    if (CurrentWeapon == AmmoEmptyWeapon)
    {
        //case if weapon call that's clip is empty
        ChangeClip();
    }
    else
    {
        for (const auto Weapon: Weapons)
        {
            if (Weapon == AmmoEmptyWeapon)
            {
                Weapon->ChangeClip();
            }
        }
    }
}

void USTUWeaponComponent::ChangeClip()
{
    if (!CanReload() || !CurrentWeapon) return;
    if (GetOwner() && !GetOwner()->HasAuthority()) return;  // Server only: ammo and anim
    CurrentWeapon->StopFire();
    CurrentWeapon->ChangeClip();
    ReloadAnimInProgress = true;
    PlayAnimMontage(CurrentReloadAnimMontage);
    MulticastPlayReloadAnim(CurrentReloadAnimMontage);
}

void USTUWeaponComponent::MulticastPlayEquipAnim_Implementation()
{
    // Clients play equip anim (server already did)
    if (GetOwner() && !GetOwner()->HasAuthority())
    {
        EquipAnimInProgress = true;
        PlayAnimMontage(EquipAnimMontage);
    }
}

void USTUWeaponComponent::MulticastPlayReloadAnim_Implementation(UAnimMontage* ReloadMontage)
{
    if (const auto Character = Cast<ASTUBaseCharacter>(GetOwner()))
    {
        Character->PlayPlayerReloadSound();
    }

    // Clients play reload anim (server already did)
    if (GetOwner() && !GetOwner()->HasAuthority() && ReloadMontage)
    {
        ReloadAnimInProgress = true;
        PlayAnimMontage(ReloadMontage);
    }
}

bool USTUWeaponComponent::GetCurrentWeaponUIData(FWeaponUIData& UIData) const
{
    if (CurrentWeapon)
    {
        UIData = CurrentWeapon->GetUIData();
        return true;
    }
    return false;
}

bool USTUWeaponComponent::GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const
{
    if (CurrentWeapon)
    {
        AmmoData = CurrentWeapon->GetAmmoData();
        return true;
    }
    return false;
}

bool USTUWeaponComponent::TryToAddAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType, int32 ClipsAmount)
{
    for (const auto Weapon: Weapons)
    {
        if (Weapon && Weapon->IsA(WeaponType))
        {
            return Weapon->TryToAddAmmo(ClipsAmount);
        }
    }
    return false;
}

bool USTUWeaponComponent::NeedAmmo(TSubclassOf<ASTUBaseWeapon> WeaponType)
{
    for (const auto Weapon : Weapons)
    {
        if (Weapon && Weapon->IsA(WeaponType))
        {
            return !Weapon->IsAmmoFull();
        }
    }
    return false;
}

void USTUWeaponComponent::Zoom(bool IsEnabled) 
{
    if(CurrentWeapon)
    {
        CurrentWeapon->Zoom(IsEnabled);
    }
}