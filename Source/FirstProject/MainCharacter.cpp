// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Weapon.h"

// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// �����ת��ʱ�򣬽�ɫ��Ҫ������ת
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// ����ۣ� ��ײʱ�Զ��ƽ����
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;	// ����������
	CameraBoom->bUsePawnControlRotation = true;		// ����Controller��ת

	// ���ý�����ײ���С
	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// �����������������ϣ���������������ɫ��ת
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
	bLMBDown = false;

	// initialize Status
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSpringStamina = 50.f;
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

	float DeltaStamina = StaminaDrainRate * DeltaTime;
	switch (StaminaStatus)
	{
	case EStaminaStatus::ESS_Normal:
		if (bShiftKeyDown) {
			if (Stamina - DeltaStamina <= MinSpringStamina) {
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else {
				Stamina -= DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Spring);
		}
		else {
			if (Stamina + DeltaStamina >= MaxStamina) {
				Stamina = MaxStamina;
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_BelowMinimum:
		if (bShiftKeyDown) {
			if (Stamina - DeltaStamina <= 0) {
				Stamina = 0;
				SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			else {
				Stamina -= DeltaStamina;
				SetMovementStatus(EMovementStatus::EMS_Spring);
			}
		}
		else {
			if (Stamina + DeltaStamina >= MinSpringStamina) {
				Stamina += DeltaStamina;
				SetStaminaStatus(EStaminaStatus::ESS_Normal);
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
		break;
	case EStaminaStatus::ESS_Exhausted:
		if (bShiftKeyDown) {
			Stamina = 0;
		}
		else {
			Stamina += DeltaStamina;
			SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	case EStaminaStatus::ESS_ExhaustedRecovering:
		if (Stamina + DeltaStamina >= MinSpringStamina) {
			SetStaminaStatus(EStaminaStatus::ESS_Normal);
			Stamina += DeltaStamina;
		}
		else {
			Stamina += DeltaStamina;
		}
		SetMovementStatus(EMovementStatus::EMS_Normal);
		break;
	default:
		;
	}
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Spring", IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction("Spring", IE_Released, this, &AMainCharacter::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMainCharacter::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMainCharacter::LMBUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Lookup", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookupRate", this, &AMainCharacter::LookUpRate);
}

void AMainCharacter::MoveForward(float fValue) {
	if (Controller != nullptr && fValue != 0.f) {
		// �ҵ�ǰ����
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, fValue);
	}
}

void AMainCharacter::MoveRight(float fValue) {
	if (Controller != nullptr && fValue != 0.f) {
		// �ҵ�ǰ����
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
		if (EquipWeapon != nullptr) {
			GetCharacterMovement()->MaxWalkSpeed = RunningSpeed / 2.;
		}
		else {
			GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
		}
	}
}

void AMainCharacter::ShiftKeyDown() {
	bShiftKeyDown = true;
}

void AMainCharacter::ShiftKeyUp() {
	bShiftKeyDown = false;
}

void AMainCharacter::LMBDown() {
	bLMBDown = true;
	if (ActiveOverlapWeapon) {
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlapWeapon);
		if (Weapon) {
			Weapon->EquipWeapon(this);
		}
	}
}

void AMainCharacter::LMBUp() {
	bLMBDown = false;
}

void AMainCharacter::SetEquipWeapon(AWeapon* WeaponToSet) {
	if (EquipWeapon) {
		EquipWeapon->Destroy();
	}
	EquipWeapon = WeaponToSet;
}