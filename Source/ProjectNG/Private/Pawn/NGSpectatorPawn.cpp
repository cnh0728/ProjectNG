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
	CameraComponent->SetupAttachment(RootComponent);
}

void ANGSpectatorPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// 서버에서 GAS있으면 초기화 
	InitHUD();
}

void ANGSpectatorPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitHUD();
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
	
}

// Called to bind functionality to input
void ANGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANGSpectatorPawn::InitHUD()
{
	ANGPlayerState* PS = GetPlayerState<ANGPlayerState>();
	ANGPlayerController* PC = Cast<ANGPlayerController>(GetController());
	
	if (!PS || !PC)	return;

	// HUD 추가
	if (PC->IsLocalController())
	{
		if (ANGHUD* MainHUD = Cast<ANGHUD>(PC->GetHUD()))
		{
			UE_LOG(LogTemp, Log, TEXT("AddToView - PC: %p, this: %p"), PC, this);
			// TODO: AttributeSet 데이터 추가
			//원래 SpectatorPawn이 GAS있었는데 지금은 필요없다고 생각돼서 뺐음. 필요하면 다시 멤버변수 선언 후 넣기
			MainHUD->InitializeHUD(PC, PS, /*AbilitySystemComponent,*/ nullptr);
		}
	}
}

