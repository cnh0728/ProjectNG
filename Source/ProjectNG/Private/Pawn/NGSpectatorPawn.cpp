// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGSpectatorPawn.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"
#include "UI/HUD/NGHUD.h"

class ANGPlayerState;
// Sets default values
ANGSpectatorPawn::ANGSpectatorPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // 줌 인/아웃은 이 값만 조절하면 됨!
	CameraBoom->bDoCollisionTest = false; // 카메라가 벽에 부딪혀서 앞으로 튀어나오는 현상 방지
	CameraBoom->bUsePawnControlRotation = false; 
	CameraBoom->bUsePawnControlRotation = false; // 컨트롤러 회전 무시
	CameraBoom->bInheritPitch = false;           // 부모 Pitch 무시
	CameraBoom->bInheritYaw = false;             // 부모 Yaw 무시
	CameraBoom->bInheritRoll = false;            // 부모 Roll 무시
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f)); // 카메라 각도 60도로 영구 고정
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	
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
	}
}

void ANGSpectatorPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitHUD();
	
	if (ANGPlayerController* PC = Cast<ANGPlayerController>(GetController()))
	{
		PC->SetControlRotation(FRotator::ZeroRotator);
	}
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

