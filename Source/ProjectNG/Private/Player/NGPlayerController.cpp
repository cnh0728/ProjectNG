// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Player/NGPlayerController.h"

#include "Components/NGPocketComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Character/NGUnitCharacter.h"
#include "Character/SelectableInterface.h"
#include "Combat/CombatManager.h"
#include "Combat/GridMapManager.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameGameMode.h"
#include "Input/NGInputComponent.h"

#include "ProjectNG/ProjectNG.h"
#include "UI/NGUnitInfoWidget.h"

class UNGInputComponent;

ANGPlayerController::ANGPlayerController() : bIsDragging(false)
{
	//커서 보이게 하는 변수
	bShowMouseCursor = true;
	bEnableClickEvents = true; 
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;

	PlayerPocket = CreateDefaultSubobject<UNGPocketComponent>("PocketComponent");

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

void ANGPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	GetMousePosition(CurrentMouseLocation.X, CurrentMouseLocation.Y);
	
	//TODO: UI랑 인게임 분기쳐서 인게임에서만 동작하도록 하면 좋을듯
	ProgressDragActor();
}

void ANGPlayerController::ProgressDragActor()
{
	if (DraggingUnit.IsValid())
	{
		FHitResult HitResult;
		if (GetHitResultUnderCursor(ECC_Map, false, HitResult))
		{
			FVector TargetLocation = HitResult.Location;

			if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>())
			{
				if (AGridMapManager* MapManager = GM->GetGridMapManager())
				{
					const FGridMap& GridMapCache = MapManager->GridMap;
					const FIntVector2 GridIndex = GridMapCache.GetCellIndex(TargetLocation);
					if (GridMapCache.IsValidIndex(GridIndex))
					{
						// TargetLocation = GridMapCache.GetWorldLocation(GridIndex);
						DraggingUnit->SetDragTargetGridIndex(GridIndex);
					}
				}
			}
			
		}
	}
}

void ANGPlayerController::HandleClickPressed(const FInputActionValue& Value)
{
	GetMousePosition(ClickStartLocation.X, ClickStartLocation.Y);
		
	PerformDrag();
}

void ANGPlayerController::HandleClickReleased(const FInputActionValue& Value)
{
	double MouseDelta = (ClickStartLocation - CurrentMouseLocation).Size();
		
	if (MouseDelta > DragThreshold)
	{
		// 클릭일땐 선택 유지, 드래그일땐 선택 취소
		ResetSelectUnit();
	}
	
	ProgressDragActor();
	
	ResetDragUnit();
}

void ANGPlayerController::UpdateUnitWidget(ANGUnitCharacter* NewUnit)
{
	//위젯이 없는 상태면 생성
	if (!UnitInfoWidgetInstance && UnitInfoWidgetClass)
	{
		UnitInfoWidgetInstance = CreateWidget<UNGUnitInfoWidget>(this, UnitInfoWidgetClass);
		if (UnitInfoWidgetInstance)
		{
			UnitInfoWidgetInstance->AddToViewport();
			UnitInfoWidgetInstance->SetVisibility(ESlateVisibility::Collapsed); //일단 숨김
		}
	}
		
	//위젯 켜고 데이터 주입
	if (IsValid(UnitInfoWidgetInstance))
	{
		UnitInfoWidgetInstance->SetTargetUnit(NewUnit);
		UnitInfoWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}
}

void ANGPlayerController::SetSelectedUnit(ANGUnitCharacter* InSelectedUnit)
{
	ResetSelectUnit();
	
	SelectedUnit = InSelectedUnit;
	
	if (SelectedUnit)
	{
		if (SelectedUnit->Implements<USelectableInterface>())
		{
			ISelectableInterface::Execute_OnSelected(SelectedUnit.Get());
		}
		
		UpdateUnitWidget(SelectedUnit);
	}
}

void ANGPlayerController::ResetSelectUnit()
{
	if (SelectedUnit)
	{
		if (SelectedUnit->Implements<USelectableInterface>())
		{
			ISelectableInterface::Execute_OnDeselected(SelectedUnit.Get());
		}
		
		if (UnitInfoWidgetInstance)
		{
			UnitInfoWidgetInstance->ClearTargetUnit();
			UnitInfoWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
		}
		
		SelectedUnit = nullptr;
	}
}

void ANGPlayerController::PerformDrag()
{
	ResetDragUnit();
	ResetSelectUnit();
	
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_SelectableUnit, false, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();

		// 디버깅용 로그 (매우 중요: 실제로 무엇이 맞았는지 확인)
		if(HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
		}
		
		if (HitActor && HitActor->Implements<USelectableInterface>())
		{
			DraggingUnit = Cast<ANGUnitCharacter>(HitActor);
			ISelectableInterface::Execute_OnDrag(HitActor);
			
			SetSelectedUnit(DraggingUnit.Get());
		}
	}
}

void ANGPlayerController::ResetDragUnit()
{
	if (DraggingUnit.IsValid())
	{
		if (DraggingUnit->Implements<USelectableInterface>())
		{
			ISelectableInterface::Execute_OnUndrag(DraggingUnit.Get());
		}
	
		DraggingUnit.Reset();
	}
}

void ANGPlayerController::Cmd_StartWave()
{
	if (HasAuthority())
	{
		if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>()){
			GM->RequestStartCombat();
            
			// 확인용 로그
			UE_LOG(LogTemp, Warning, TEXT("Cmd: Wave Started!"));
            
			// 화면에 디버그 메시지 띄우기 (선택사항)
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Wave Started!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CombatManager가 GameState에 없습니다!"));
		}
	}
}
