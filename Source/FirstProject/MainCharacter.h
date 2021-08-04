// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8 {
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Spring UMETA(DisplayName = "Spring"),

	EMX_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8 {
	/**
	 * 正常
	 */
	ESS_Normal UMETA(DisplayName = "Normal"),
	/**
	 * 在低标准下
	 */
	ESS_BelowMinimum UMETA(DisplayName = "BelowMininum"),
	/**
	 * 耗尽
	 */
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	/**
	 * 耗尽回复
	 */
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "Default")
};

UCLASS()
class FIRSTPROJECT_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	AMainCharacter();

	// AllowPrivateAccess 蓝图私有，不可以被其他蓝图访问
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	/**
	 * follow camera
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookupRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Item)
	class AWeapon* EquipWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Item)
	class AItem* ActiveOverlapWeapon;

#pragma region PlayerStats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	int32 Coins;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float SpringSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSpringStamina;

#pragma endregion PlayerStats

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminaStatus StaminaStatus;
public:
	bool bShiftKeyDown;
	
	bool bLMBDown;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void DecrementHealth(float Damage);

	void IncrementCoins(int32 Coins);

	void Die();

	void SetMovementStatus(EMovementStatus Status);

	FORCEINLINE class USpringArmComponent* GetSpringArmComponent() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetCamera() const { return FollowCamera; }
	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }
	void SetEquipWeapon(class AWeapon* WeaponToSet);
	FORCEINLINE AWeapon* GetEquipWeapon() const { return EquipWeapon; }
	FORCEINLINE void SetActiveOverlapWeapon(class AItem* ItemToSet) { ActiveOverlapWeapon = ItemToSet; }
private:
	/**
	 * 前进后退事件
	 */
	void MoveForward(float fValue);

	/**
	 * 横向移动事件
	 */
	void MoveRight(float fValue);

	void TurnAtRate(float fRate);

	void LookUpRate(float fRate);

	void ShiftKeyDown();

	void ShiftKeyUp();

	void LMBDown();

	void LMBUp();
};
