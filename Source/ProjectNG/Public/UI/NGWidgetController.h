// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NGWidgetController.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FWidgetParams
{
	GENERATED_BODY()

	FWidgetParams() {}

	FWidgetParams(APlayerController* PC, APlayerState* PS)
	: PlayerController(PC), PlayerState(PS) { }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;
};


/**
 * WidgetController for MVC.
 */
UCLASS()
class PROJECTNG_API UNGWidgetController : public UObject
{
	GENERATED_BODY()

public:

	// 사용할 데이터 셋 바인딩
	UFUNCTION(BlueprintCallable, Category = "WidgetController|Functions")
	void AssignWidgetControllerParams(const FWidgetParams& InWidgetParams);

	// 데이터가 연결되고 초기값 브로드캐스팅
	UFUNCTION(BlueprintCallable, Category = "WidgetController|Functions")
	virtual void BroadcastInitialValues();
	
	virtual void BindCallbacksToDependencies();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;
};
