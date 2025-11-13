// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Character/NGUnitCharacter.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"


// Sets default values
ANGUnitCharacter::ANGUnitCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UNGAbilitySystemComponent>(TEXT("Ability System Component"));
}

// Called when the game starts or when spawned
void ANGUnitCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();
}

void ANGUnitCharacter::InitAbilityActorInfo()
{
	// ASC 복사 및 아바타 설정
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

// Called every frame
void ANGUnitCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ANGUnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

