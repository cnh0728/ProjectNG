// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGEnemyPawn.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"
#include "ProjectNG/ProjectNG.h"


// Sets default values
ANGEnemyPawn::ANGEnemyPawn() : CurrentDistance(0.f), bCanMoving(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
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
	
}


// Called to bind functionality to input
void ANGEnemyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANGEnemyPawn::InitAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
