 // Shoot Them Up Game, All Rights Reserved.

#include "Player/STUBaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/STUCharacterMovementComponent.h"
#include "Components/STUHealthComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Controller.h"

//TODO: read about define log category
DEFINE_LOG_CATEGORY_STATIC(BaseCharacterLog, All, All);

//Sets default values in constructor 
 ASTUBaseCharacter::ASTUBaseCharacter(const FObjectInitializer& ObjInit)
     : Super(ObjInit.SetDefaultSubobjectClass<USTUCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
 {
    // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
    SpringArmComponent->SetupAttachment(GetRootComponent());
    SpringArmComponent->bUsePawnControlRotation = true;

    CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
    CameraComponent->SetupAttachment(SpringArmComponent);

    CameraComponent2 = CreateDefaultSubobject<UCameraComponent>("CameraComponent2");
    CameraComponent2->SetupAttachment(SpringArmComponent);

    //Create Health component in constructor of STUBaseCharacter
    HealthComponent = CreateDefaultSubobject<USTUHealthComponent>("HealthComponent");

    HealthTextComponent = CreateDefaultSubobject<UTextRenderComponent>("HealthTextComponent");
    HealthTextComponent->SetupAttachment(GetRootComponent());

 }

// Called when the game starts or when spawned
void ASTUBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    //macros for check that Health Component and HealthTextComponent is not null (work only in Debug and Development builds)
    check(HealthComponent);
    check(HealthTextComponent);
    check(GetCharacterMovement());

    OnHealthChanged(HealthComponent->GetHealth());
    HealthComponent->OnDeath.AddUObject(this, &ASTUBaseCharacter::OnDeath);

    //Bind a new function to the OnHealthChanged delegate and update the text component only when the value actually changes, not every frame
    HealthComponent->OnHealthChanged.AddUObject(this, &ASTUBaseCharacter::OnHealthChanged);

}

void ASTUBaseCharacter::OnHealthChanged(float Health)
{
    // Update the text component with the current health value
    HealthTextComponent->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Health)));
}

// Called every frame
void ASTUBaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASTUBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ASTUBaseCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ASTUBaseCharacter::MoveRight);
    PlayerInputComponent->BindAxis("LookUp", this, &ASTUBaseCharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnAround", this, &ASTUBaseCharacter::AddControllerYawInput);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASTUBaseCharacter::Jump);
    PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ASTUBaseCharacter::OnStartRunning);
    PlayerInputComponent->BindAction("Run", IE_Released, this, &ASTUBaseCharacter::OnStopRunning);

}

void ASTUBaseCharacter::MoveForward(float Amount)
{
    IsMovingForward = Amount > 0.0f;
    if (Amount == 0.0f) return;
    AddMovementInput(GetActorForwardVector(), Amount);
}

void ASTUBaseCharacter::MoveRight(float Amount)
{
    if (Amount == 0.0f) return;
    AddMovementInput(GetActorRightVector(), Amount);
}

void ASTUBaseCharacter::OnStartRunning()
{
    WantsToRun = true;
}

void ASTUBaseCharacter::OnStopRunning()
{
    WantsToRun = false;
}

 bool ASTUBaseCharacter::IsRunning() const
{
    return WantsToRun && IsMovingForward && !GetVelocity().IsZero();
}

 float ASTUBaseCharacter::GetMovementDirection() const
 {
     //If the speed is zero, return 0.0f since there is no movement
     if (GetVelocity().IsZero()) return 0.0f;

     //define the normalized velocity vector
     const auto VelocityNormal = GetVelocity().GetSafeNormal();
     //define variable for angle (scalar product) between GetActorForwardVector and VelocityNormal, return Radians
     const auto AngleBetween = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));
     //define variable (vector product) to calculate vector product
     const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);
     // Convert the angle from radians to degrees
     const auto Degrees = FMath::RadiansToDegrees(AngleBetween);

     // Return the angle in degrees multiplied by the sign of the Z component of the vector product,
     // if the vector product is not zero; otherwise we return the angle in degrees without changing
     return  CrossProduct.IsZero() ? Degrees : Degrees * FMath::Sign(CrossProduct.Z);
 }

void ASTUBaseCharacter::OnDeath()
 {
    UE_LOG(BaseCharacterLog, Display, TEXT("Player %s is dead"), *GetName());

    PlayAnimMontage(DeathAnimMontage);

    GetCharacterMovement()->DisableMovement();

    SetLifeSpan(5.0f);

    if (Controller)
    {
        Controller->ChangeState(NAME_Spectating);
    }
 }