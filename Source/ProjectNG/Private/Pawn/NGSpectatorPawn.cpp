// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGSpectatorPawn.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Input/NGInputComponent.h"
#include "Player/NGPlayerState.h"


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

	// 입력 매핑 컨텍스트 추가
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (!PlayerController) return;

	if (
		UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())
	) {
		if (ensure(InputMappingContext.IsValid()))
		{
			Subsystem->AddMappingContext(InputMappingContext.LoadSynchronous(), 0);
		}
	}
}

// Called to bind functionality to input
void ANGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input 관련 로직 Mapping할 때 사용
	[[maybe_unused]] UNGInputComponent* IC = CastChecked<UNGInputComponent>(PlayerInputComponent);
}

// Called every frame
void ANGSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANGSpectatorPawn::InitAbilityActorInfo()
{
	ANGPlayerState* PS = GetPlayerState<ANGPlayerState>();
	check(PS);

	// ASC 복사 및 아바타 설정
	AbilitySystemComponent = PS->GetAbilitySystemComponent();
	AbilitySystemComponent->InitAbilityActorInfo(PS, this);
}

