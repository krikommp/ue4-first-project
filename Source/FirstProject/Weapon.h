// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8 {
	EWS_Pickup		UMETA(DisplayName = "Pickup"),
	EWS_Equipped	UMETA(DisplayName = "Equipped"),

	EWS_MAX			UMETA(DisplayName = "Default")
};

UCLASS()
class FIRSTPROJECT_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SkeletalMesh")
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item | Sound")
	class USoundCue* OnEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Particles")
	bool bWeaponParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item | Weapon")
	EWeaponState WeaponState;
public:
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	
	void EquipWeapon(class AMainCharacter* Character);

	FORCEINLINE void SetWeaponState(EWeaponState State) { WeaponState = State; }
	FORCEINLINE EWeaponState GetWeaponState() const { return WeaponState; }
};
