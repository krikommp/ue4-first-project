// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 相机旋转的时候，角色不要跟着旋转
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// 相机臂， 碰撞时自动推进相机
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;	// 相机跟随距离
	CameraBoom->bUsePawnControlRotation = true;		// 跟着Controller旋转

	// 设置胶囊碰撞体大小
	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// 将相机附加在相机臂上，让相机臂来适配角色旋转
	FollowCamera->bUsePawnControlRotation = false;

	BaseTurnRate = 65.f;
	BaseLookupRate = 65.f;

	// Player Stats
	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 350.f;
	Stamina = 120.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SpringSpeed = 900.f;

	bShiftKeyDown = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Spring", IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction("Spring", IE_Released, this, &AMainCharacter::ShiftKeyUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Lookup", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookupRate", this, &AMainCharacter::LookUpRate);
}

void AMainCharacter::MoveForward(float fValue) {
	if (Controller != nullptr && fValue != 0.f) {
		// 找到前方向
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, fValue);
	}
}

void AMainCharacter::MoveRight(float fValue) {
	if (Controller != nullptr && fValue != 0.f) {
		// 找到前方向
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, fValue);
	}
}

void AMainCharacter::TurnAtRate(float fRate) {
	AddControllerYawInput(fRate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpRate(float fRate) {
	AddControllerPitchInput(fRate * BaseLookupRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::DecrementHealth(float Damage) {
	if (Health - Damage <= 0.f) {
		Health -= Damage;
		Die();
	}
	else {
		Health -= Damage;
	}
}

void AMainCharacter::IncrementCoins(int32 Amount) {
	Coins += Amount;
}

void AMainCharacter::Die() {

}

void AMainCharacter::SetMovementStatus(EMovementStatus Status) {
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Spring) {
		GetCharacterMovement()->MaxWalkSpeed = SpringSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMainCharacter::ShiftKeyDown() {
	bShiftKeyDown = true;
}

void AMainCharacter::ShiftKeyUp() {
	bShiftKeyDown = false;
}
