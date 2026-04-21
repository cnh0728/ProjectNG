// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/NGPawnBase.h"
#include "NGEnemyPawn.generated.h"

class UWidgetComponent;
class USplineComponent;

UCLASS()
class PROJECTNG_API ANGEnemyPawn : public ANGPawnBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANGEnemyPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void MoveOnSpline(float DeltaTime);

	// Called to bind functionality to input
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
