// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NGPlayerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

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
	
	void HandleClickPressed(const FInputActionValue& Value);
	void HandleClickReleased(const FInputActionValue& Value);

	void PerformSingleSelect();
	void PerformDragSelect();
	
	// Enhanced Input 관련
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ClickInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MousePositionInputAction;

	//상태 관리 변수
	bool bIsDragging;

	FVector2D DragStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Selection")
	TArray<TObjectPtr<AActor>> SelectedUnits;
	
};
