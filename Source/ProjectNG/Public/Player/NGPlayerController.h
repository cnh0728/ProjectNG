// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NGPlayerController.generated.h"

class UNGUnitInfoWidget;
class ANGUnitCharacter;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class UNGPocketComponent;

/**
 * 플레이어의 입력을 처리하는 Class
 */

UCLASS()
class PROJECTNG_API ANGPlayerController : public APlayerController
{
	GENERATED_BODY()

	ANGPlayerController();
	
protected:
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaTime) override;

/*************************************/
/*				피킹 관련			 */
/*************************************/
public:
	FVector2D GetStartMousePosition() const {return ClickStartLocation;}
	FVector2D GetCurrentMousePosition() const {return CurrentMouseLocation;}

protected:
	virtual void ProgressDragActor();

	void HandleClickPressed(const FInputActionValue& Value);
	void HandleClickReleased(const FInputActionValue& Value);
	
	void UpdateUnitWidget(ANGUnitCharacter* NewUnit);

	void SetSelectedUnit(ANGUnitCharacter* InSelectedUnit);
	void ResetSelectUnit();
	
	void PerformDrag();
	void ResetDragUnit();
	
	// Enhanced Input 관련
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ClickInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MousePositionInputAction;

	//상태 관리 변수
	uint8 bIsDragging : 1;
	
	const float DragThreshold = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	float DragHeightOffset = 50.0f;

	FVector2D ClickStartLocation;
	FVector2D CurrentMouseLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	TWeakObjectPtr<ANGUnitCharacter> DraggingUnit;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	TObjectPtr<ANGUnitCharacter> SelectedUnit;
/*************************************/
/*				리롤 관련			 */
/*************************************/
public:
	UNGPocketComponent* GetPlayerPocket() { return PlayerPocket; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Pocket")
	TObjectPtr<UNGPocketComponent> PlayerPocket;
	
/*************************************/
/*				UI					 */
/*************************************/
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNGUnitInfoWidget> UnitInfoWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UNGUnitInfoWidget> UnitInfoWidgetInstance;
	
/*************************************/
/*				Debug				 */
/*************************************/
	
public:
	UFUNCTION(Exec)
	void Cmd_StartWave();
	
};
