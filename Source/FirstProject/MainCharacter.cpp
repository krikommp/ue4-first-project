// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

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
	bLMBDown = false;

	// initialize Status
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSpringStamina = 50.f;

	AttackIndex = 0;

	CacheMontage = nullptr;
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
	if (Controller != nullptr && fValue != 0.f && (!bAttacking)) {
		// 找到前方向
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, fValue);
	}
}

void AMainCharacter::MoveRight(float fValue) {
	if (Controller != nullptr && fValue != 0.f && (!bAttacking)) {
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
	else if (EquipWeapon) {
		Attack();
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

void AMainCharacter::Attack() {
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance) {
		if (CurrentMontage == nullptr) {
			AttackIndex = 0;
			bAttacking = true;
			CurrentMontage = CombatMontages[AttackIndex];
			AnimInstance->Montage_Play(CurrentMontage);
		}
		else {
			if (AnimInstance->Montage_IsPlaying(CurrentMontage)) {
				auto sectionName = AnimInstance->Montage_GetCurrentSection(CurrentMontage);
				if (sectionName.ToString().Contains("ComboWindow")) {
					// 连击缓存窗口，这时候应该缓存攻击事件，等待当前攻击结束，播放缓存的攻击动作
					if (CacheMontage == nullptr) {
						// 判断是否已经有缓存了，如果有就不需要缓存，等待动画结束播放即可
						// 如果没有缓存，就移动到下一个动作，并更新缓存
						++AttackIndex;
						if (AttackIndex >= CombatMontages.Num()) {
							AttackIndex = 0;
						}
						UE_LOG(LogTemp, Warning, TEXT("Combo Window Next Index = %d"), AttackIndex);
						CacheMontage = CombatMontages[AttackIndex];
					}
				}
				else if (sectionName.ToString().Contains("FadeOut")) {
					// 动作后摇窗口，此时如果点击，那么就会立即播放下一个动作，取消前一个动作的后摇
					if (CacheMontage == nullptr) {
						++AttackIndex;
						if (AttackIndex >= CombatMontages.Num()) {
							AttackIndex = 0;
						}
						CurrentMontage = CombatMontages[AttackIndex];
						bAttacking = true;
						AnimInstance->Montage_Play(CurrentMontage);
					}
				}
			}
		}
	}
}

void AMainCharacter::SaveAttack() {
	bAttacking = false;
}

void AMainCharacter::ResetAttack() {
	if (CacheMontage) {
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance) {
			bAttacking = true;
			AnimInstance->Montage_Play(CacheMontage);
			CurrentMontage = CacheMontage;
			CacheMontage = nullptr;
		}
	}
	else {
		CurrentMontage = nullptr;
		bAttacking = false;
		AttackIndex = 0;
	}
}