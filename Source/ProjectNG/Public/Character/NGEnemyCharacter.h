// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "NGCharacterBase.h"
#include "NGEnemyCharacter.generated.h"

class UWidgetComponent;
class USplineComponent;

UCLASS()
class PROJECTNG_API ANGEnemyCharacter : public ANGCharacterBase
{
	GENERATED_BODY()

public:
	ANGEnemyCharacter();

public:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	void MoveOnSpline(float DeltaTime);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void InitPatrolPath(USplineComponent* SplineComponent, FVector InLocationOffset = FVector::ZeroVector);

private:
	virtual void InitAbilityActorInfo() override;
	
private:
	UPROPERTY()
	TObjectPtr<USplineComponent> PathSplineComponent;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed; //이동속도
	
	float CurrentDistance;
	
	bool bCanMoving;
};
