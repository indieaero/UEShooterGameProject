// Shoot Them Up Game, All Rights Reserved.

#include "Dev/STUDevBoxDamageActor.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ASTUDevBoxDamageActor::ASTUDevBoxDamageActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
}

void ASTUDevBoxDamageActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Replicated copy on clients: no debug draw (DrawDebug* ignores actor visibility) and no damage — server is authoritative.
	if (!HasAuthority())
	{
		return;
	}

	if (bDrawDebug)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), HalfExtents, FQuat::Identity, BoxColor);
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> Overlapping;
	UKismetSystemLibrary::BoxOverlapActors(this, GetActorLocation(), HalfExtents, ObjectTypes, nullptr, TArray<AActor*>(), Overlapping);

	for (AActor* const Target : Overlapping)
	{
		if (!Target || Target == this)
		{
			continue;
		}
		// ApplyDamage uses generic FDamageEvent → only OnTakeAnyDamage; this project applies HP in OnTakePointDamage / OnTakeRadialDamage.
		const FVector ShotDir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FHitResult Hit;
		Hit.Location = Target->GetActorLocation();
		Hit.ImpactPoint = Hit.Location;
		Hit.ImpactNormal = ShotDir.IsNearlyZero() ? FVector::UpVector : -ShotDir;
		Hit.Component = Cast<UPrimitiveComponent>(Target->GetRootComponent());
		UGameplayStatics::ApplyPointDamage(Target, Damage, ShotDir, Hit, nullptr, this, DamageType);
	}
}
