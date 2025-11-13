// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Character/NGSpectatorCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Input/NGInputComponent.h"
#include "Player/NGPlayerState.h"


// Sets default values
ANGSpectatorCharacter::ANGSpectatorCharacter()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
}

void ANGSpectatorCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 서버에서 GAS 초기화
	InitAbilityActorInfo();
}

void ANGSpectatorCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라이언트에서 GAS 초기화
	InitAbilityActorInfo();
}

// Called when the game starts or when spawned
void ANGSpectatorCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called to bind functionality to input
void ANGSpectatorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{

}

// Called every frame
void ANGSpectatorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANGSpectatorCharacter::InitAbilityActorInfo()
{
	ANGPlayerState* PS = GetPlayerState<ANGPlayerState>();
	check(PS);

	// ASC 복사 및 아바타 설정
	AbilitySystemComponent = PS->GetNGAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);
}
