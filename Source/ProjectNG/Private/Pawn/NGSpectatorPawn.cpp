// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGSpectatorPawn.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"
#include "UI/HUD/NGHUD.h"

// Sets default values
ANGSpectatorPawn::ANGSpectatorPawn()
{
	SetReplicates(true);
	PrimaryActorTick.bCanEverTick = true;
    
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	if (GetCollisionComponent())
	{
		CameraComponent->SetupAttachment(GetCollisionComponent());
	}
	else
	{
		CameraComponent->SetupAttachment(RootComponent);
	}
    
	CameraComponent->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	CameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f)); 
	CameraComponent->bUsePawnControlRotation = false; 

	// 밀려남 차단
	if (USphereComponent* CollisionComp = GetCollisionComponent())
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
    
	bFindCameraComponentWhenViewTarget = true;
}

void ANGSpectatorPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// 서버에서 GAS있으면 초기화 
	InitHUD();
	
	if (ANGPlayerController* PC = Cast<ANGPlayerController>(NewController))
	{
		PC->SetViewTarget(this);
		PC->SetControlRotation(FRotator::ZeroRotator);
		
		//PossessedBy가 PostLogin보다 빠르기 때문에 다른 타이밍에서 GridManager세팅
	}
}

void ANGSpectatorPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitHUD();
	
	// FocusOnMyGrid();
}

void ANGSpectatorPawn::OnRep_Controller()
{
	Super::OnRep_Controller();
	
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

void ANGSpectatorPawn::PossessCamera(const FTransform& CameraTransform)
{
	//Client함수
	if (HasAuthority())	return;
	
	SetActorTransform(CameraTransform);
}

void ANGSpectatorPawn::Client_PossessCamera_Implementation(const FTransform& CameraTransform)
{
	PossessCamera(CameraTransform);
}

