// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Player/NGPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Character/SelectableInterface.h"
#include "Input/NGInputComponent.h"
#include "Kismet/GameplayStatics.h"

#include "ProjectNG/ProjectNG.h"

class UNGInputComponent;

ANGPlayerController::ANGPlayerController() : bIsDragging(false)
{
	//커서 보이게 하는 변수
	bShowMouseCursor = true;
	bEnableClickEvents = true; 
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default; 
}

void ANGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext.Get(), 0);
			}
		}
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ANGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// InputComponent를 EnhancedInputComponent로 캐스팅합니다.
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// IA_Click 액션을 C++ 함수와 바인딩합니다.
		if (ClickInputAction)
		{
			EnhancedInputComponent->BindAction(ClickInputAction, ETriggerEvent::Started, this, &ThisClass::HandleClickPressed);
			EnhancedInputComponent->BindAction(ClickInputAction, ETriggerEvent::Completed, this, &ThisClass::HandleClickReleased);
		}
	}
}

void ANGPlayerController::HandleClickPressed(const FInputActionValue& Value)
{
	bIsDragging = true;
	GetMousePosition(DragStartLocation.X, DragStartLocation.Y);

	for (AActor* SelectedUnit: SelectedUnits)
	{
		if (SelectedUnit && SelectedUnit->Implements<USelectableInterface>())
		{
			ISelectableInterface::Execute_OnDeselected(SelectedUnit);
		}
	}
	SelectedUnits.Empty();
}

void ANGPlayerController::HandleClickReleased(const FInputActionValue& Value)
{
	bIsDragging = false;

	FVector2D DragEndLocation;
	GetMousePosition(DragEndLocation.X, DragEndLocation.Y);

	if (FVector2D::DistSquared(DragStartLocation, DragEndLocation) < 100.f)
	{
		PerformSingleSelect();
	}else
	{
		PerformDragSelect();
	}
}

void ANGPlayerController::PerformSingleSelect()
{
	FHitResult HitResult;

	// if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	if (GetHitResultUnderCursor(ECC_Selectable, false, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();

		// 디버깅용 로그 (매우 중요: 실제로 무엇이 맞았는지 확인)
		// if(HitActor)
		// {
		// 	UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
		// }
		
		if (HitActor && HitActor->Implements<USelectableInterface>())
		{
			SelectedUnits.Add(HitActor);
			ISelectableInterface::Execute_OnSelected(HitActor);
		}
	}
}

void ANGPlayerController::PerformDragSelect()
{
	//TODO: 드래그 영역 그리기

	TArray<AActor*> AllSelectableActors;

	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USelectableInterface::StaticClass(), AllSelectableActors);

	FVector2D CurrentMouseLocation;
	GetMousePosition(CurrentMouseLocation.X, CurrentMouseLocation.Y);

	FBox2D SelectionRectangle(ForceInit);
	SelectionRectangle += DragStartLocation;
	SelectionRectangle += CurrentMouseLocation;

	for (AActor* SelectableActor : AllSelectableActors)
	{
		FVector ActorLocation = SelectableActor->GetActorLocation();
		FVector2D ScreenLocation;

		if (ProjectWorldLocationToScreen(ActorLocation, ScreenLocation))
		{
			if (SelectionRectangle.IsInside(ScreenLocation))
			{
				SelectedUnits.Add(SelectableActor);
				ISelectableInterface::Execute_OnSelected(SelectableActor);
			}
		}
	}
	
}
