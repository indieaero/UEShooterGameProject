// Shoot Them Up Game, All Rights Reserved.

#include "Pickups/STUHealthPickup.h"

DEFINE_LOG_CATEGORY_STATIC(LogHealthPickup, All, All)

bool ASTUHealthPickup::GivePickupTo(APawn* PlayerPawn)
{
    UE_LOG(LogHealthPickup, Display, TEXT("Health was taken"));
    return true;
}
