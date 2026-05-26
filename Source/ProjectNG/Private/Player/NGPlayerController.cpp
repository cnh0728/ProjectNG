// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Player/NGPlayerController.h"

#include "Components/NGPocketComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/CombatManagerComponent.h"
#include "Pawn/NGUnitPawn.h"
#include "Pawn/SelectableInterface.h"
#include "Core/NGSpawnHelper.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Input/NGInputComponent.h"
#include "Player/NGPlayerState.h"

#include "ProjectNG/ProjectNG.h"
#include "UI/NGUnitInfoWidget.h"

ANGPlayerController::ANGPlayerController() : DragThreshold(10.f), DragHeightOffset(20.f), DragInterpSpeed(20.f)
{
	UE_LOG(LogTemp, Warning, TEXT("---------------PC Created!---------------"));
	UE_LOG(LogTemp, Warning, TEXT("PC Addr: %p"), this);
}

ANGPlayerController::~ANGPlayerController()
{
	UE_LOG(LogTemp, Warning, TEXT("---------------PC Destroyed!---------------"));
}

void ANGPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	SetIgnoreLookInput(true);
	
	bShowMouseCursor = true;
	bEnableClickEvents = true; 
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	
	CurrentClickTraceChannel = ECC_SelectableUnit;
	
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewYawMin = -179.9f;
		PlayerCameraManager->ViewYawMax = 179.9f;
		PlayerCameraManager->ViewPitchMin = -89.9f;
		PlayerCameraManager->ViewPitchMax = 89.9f;
	}
	
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
	
	if (IsLocalController() && DraggingUnit.IsValid())
	{
		PerformDragUpdate(DeltaTime);
	}
	
	if (bShowDebugGrid)
	{
		if (ANGPlayerState* PS = GetPlayerState<ANGPlayerState>())
		{
			FGridAddress CombatGridAddress(FIntVector2::ZeroValue, EGridType::Combat, PS);
			UGridMapHelper::DrawDebugGrid(this, CombatGridAddress);
			FGridAddress WaitGridAddress(FIntVector2::ZeroValue, EGridType::Wait, PS);
			UGridMapHelper::DrawDebugGrid(this, WaitGridAddress);
		}
	}
}

void ANGPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
}

void ANGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

}

void ANGPlayerController::PerformDragUpdate(float DeltaTime)
{
	if (!HasAuthority())
	{
		if (DraggingUnit.IsValid())
		{
			FHitResult HitResult;
			
			if (GetHitResultUnderCursor(ECC_Map, false, HitResult))
			{
				//TODO: HitResult 그리드 하이라이트
				
				FVector CurrentLocation = DraggingUnit->GetActorLocation();
				
				FVector TargetLocation = HitResult.Location;
				TargetLocation.Z += DragHeightOffset;
				
				FVector SmoothedLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, DragInterpSpeed);
				
				DraggingUnit->SetActorLocation(SmoothedLocation);
			}
		}
	}
}

void ANGPlayerController::SetHoveringUnit(ANGPawnBase* InHoveringPawn)
{
	HoveringUnit = InHoveringPawn;
}

ANGPawnBase* ANGPlayerController::GetHoveringUnit() const
{
	return HoveringUnit.Get();
}

void ANGPlayerController::HandleClickPressed(const FInputActionValue& Value)
{
	GetMousePosition(ClickStartLocation.X, ClickStartLocation.Y);
	
	PerformDrag();
}

void ANGPlayerController::HandleClickReleased(const FInputActionValue& Value)
{
	if (DraggingUnit.IsValid())
	{
		double MouseDelta = (ClickStartLocation - CurrentMouseLocation).Size();
	
		if (MouseDelta > DragThreshold)
		{
			//많이움직여서 옮긴판정
			FHitResult HitResult;
				
			if (GetHitResultUnderCursor(ECC_Map, false, HitResult))
			{			
				ANGPawnBase* Unit = DraggingUnit.Get();
				
				FVector TargetLocation = HitResult.Location;
				
				Unit->TryMoveTo(TargetLocation);
			}
		}else
		{
			SetSelectedUnit(DraggingUnit.Get());
		}
		
		ResetDragUnit();
	}
}

void ANGPlayerController::UpdateUnitWidget(ANGPawnBase* NewUnit)
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

void ANGPlayerController::SetSelectedUnit(ANGPawnBase* InSelectedUnit)
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
	if (!IsLocalController())	return;
	
	ResetDragUnit();
	ResetSelectUnit();
	
	if (HoveringUnit.IsValid())
	{
		//다른 유저거면 return
		if (HoveringUnit->GetOwner() != this)
		{
			return;
		}

		if (HoveringUnit->Implements<USelectableInterface>())
		{
			// UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
			
			DraggingUnit = Cast<ANGPawnBase>(HoveringUnit);
			ISelectableInterface::Execute_OnDrag(HoveringUnit.Get());
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

void ANGPlayerController::Server_RequestBuyUnit_Implementation(FName UnitName)
{
	if (ANGPlayerState* PS = GetPlayerState<ANGPlayerState>())
	{
		if (UNGSpawnHelper::SpawnUnitPawn(this, UnitName))
		{
			UNGPocketComponent* PlayerPocket = PS->GetPlayerPocket();
			PlayerPocket->AddUnitToBuyingPocket(UnitName);
			UE_LOG(LogTemp, Display, TEXT("BuyUnitFromPocket Success"));
		}
	}
}

UNGPocketComponent* ANGPlayerController::GetPlayerPocket() const
{
	if (ANGPlayerState* PS = GetPlayerState<ANGPlayerState>())
	{
		return PS->GetPlayerPocket();
	}
	
	return nullptr;
}

void ANGPlayerController::Server_RequestStartCombat_Implementation()
{
	if (HasAuthority())
	{
		if (ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>())
		{
			GM->RequestStartCombat(this);
            
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

void ANGPlayerController::Server_RequestStopCombat_Implementation()
{
	if (HasAuthority())
	{
		if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
		{
			if (UCombatManagerComponent* CMC = GS->GetCombatManagerComponent())
			{
				CMC->FinishCombat();
			}
		}
	}
}

void ANGPlayerController::Cmd_StartCombat()
{
	Server_RequestStartCombat();
}

void ANGPlayerController::Cmd_FinishCombat()
{
	Server_RequestStopCombat();
}

void ANGPlayerController::Cmd_ToggleDebugGrid()
{
	bShowDebugGrid = !bShowDebugGrid;
}
