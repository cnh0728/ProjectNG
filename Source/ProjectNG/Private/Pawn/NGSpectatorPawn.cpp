// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGSpectatorPawn.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"
#include "UI/HUD/NGHUD.h"


class ANGPlayerState;
// Sets default values
ANGSpectatorPawn::ANGSpectatorPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	
}

void ANGSpectatorPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// 서버에서 GAS 초기화
	InitAbilityActorInfo();
}

void ANGSpectatorPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// 클라이언트에서 GAS 초기화
	InitAbilityActorInfo();
}

// Called when the game starts or when spawned
void ANGSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ANGSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANGSpectatorPawn::OnRep_Controller()
{
	Super::OnRep_Controller();
	
	//클라이언트에서도 초기화
	InitAbilityActorInfo();
}

// Called to bind functionality to input
void ANGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANGSpectatorPawn::InitAbilityActorInfo()
{
	ANGPlayerState* PS = GetPlayerState<ANGPlayerState>();
	ANGPlayerController* PC = Cast<ANGPlayerController>(GetController());
	
	if (!PS || !PC)	return;
	
	// ASC 복사 및 아바타 설정
	AbilitySystemComponent = PS->GetNGAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);

	// HUD 추가
	if (PC->IsLocalController())
	{
		if (ANGHUD* MainHUD = Cast<ANGHUD>(PC->GetHUD()))
		{
			// TODO: AttributeSet 데이터 추가
			MainHUD->InitializeHUD(PC, PS, AbilitySystemComponent, nullptr);
		}
	}
}

