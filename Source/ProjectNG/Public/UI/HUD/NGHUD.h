// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NGHUD.generated.h"

struct FWidgetParams;
class UMainWidgetController;
class UNGUserWidget;
class UAbilitySystemComponent;
class UAttributeSet;

/**
 * 
 */
UCLASS()
class PROJECTNG_API ANGHUD : public AHUD
{
	GENERATED_BODY()

public:
	UMainWidgetController* GetMainWidgetController(const FWidgetParams& WidgetControllerParams);

	void InitializeHUD(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:

	//~ Begin Main Widget
	UPROPERTY()
	TObjectPtr<UNGUserWidget> MainWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY()
	TObjectPtr<UMainWidgetController> MainWidgetController;

	// 새롭게 생성 시 필요한 StaticClass
	UPROPERTY(EditAnywhere)
	TSubclassOf<UMainWidgetController> MainWidgetControllerClass;
	//~ End Main Widget
};
