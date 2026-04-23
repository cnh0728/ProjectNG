// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGEnemyPawn.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"
#include "ProjectNG/ProjectNG.h"


// Sets default values
ANGEnemyPawn::ANGEnemyPawn() : MoveSpeed(300.f), CurrentDistance(0.f), bCanMoving(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Enemy);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
}

// Called when the game starts or when spawned
void ANGEnemyPawn::BeginPlay()
{
	Super::BeginPlay();
	
	InitAbilityActorInfo();
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UNGAttributeSet::GetHealthAttribute())
	.AddUObject(this, &ANGEnemyPawn::OnHealthChanged);
}

// Called every frame
void ANGEnemyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	MoveOnSpline(DeltaTime);
}

void ANGEnemyPawn::MoveOnSpline(float DeltaTime)
{
	if (!IsValid(PathSplineComponent))	return;
	
	CurrentDistance += MoveSpeed * DeltaTime;
	
	float TotalLength = PathSplineComponent->GetSplineLength();
	
	if (CurrentDistance > TotalLength)
	{
		//float끼리는 모듈러 연산이 안됨
		CurrentDistance -= TotalLength;
	}
	
	FVector NewLocation = PathSplineComponent->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
	FRotator NewRotation = PathSplineComponent->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
	
	//콜리전 위치 조정
	NewLocation += GetHalfCapsule();
	
	SetActorLocationAndRotation(NewLocation, NewRotation);
}

// Called to bind functionality to input
void ANGEnemyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANGEnemyPawn::InitPatrolPath(USplineComponent* SplineComponent, const FVector InLocationOffset)
{
	if (!IsValid(SplineComponent))	return;

	PathSplineComponent = SplineComponent;
	CurrentDistance = 0.f;
	bCanMoving = true;
}

void ANGEnemyPawn::InitAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
