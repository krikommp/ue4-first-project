// Fill out your copyright notice in the Description page of Project Settings.


#include "CollliderMovementComponent.h"

void UCollliderMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime)) {
		return;
	}
	// �����ƶ��ľ���
	// ����ÿ�ΰ�������ֵ
	FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f);

	if (!DesiredMovementThisFrame.IsNearlyZero()) {
		// �ƶ�Pawn
		FHitResult Hit;
		// sweep true ������ײ���
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);
		// �Ƿ�����ײ
		// ������ײ������ǽ�滬��
		if (Hit.IsValidBlockingHit()) {
			SlideAlongSurface(DesiredMovementThisFrame, 1.0f - Hit.Time, Hit.Normal, Hit);
			UE_LOG(LogTemp, Warning, TEXT("Valid Block Sometings"));
		}
	}
}