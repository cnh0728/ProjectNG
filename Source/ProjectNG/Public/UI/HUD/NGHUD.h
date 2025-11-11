// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NGHUD.generated.h"

struct FWidgetParams;
class URollShopWidgetController;
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
	URollShopWidgetController* GetRollShopWidgetController(const FWidgetParams& WidgetControllerParams);

	void InitializeHUD(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:

	//~ Begin Main Widget
	UPROPERTY()
	TObjectPtr<UNGUserWidget> MainWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> MainWidgetClass;
	//~ End Main Widget

	//~ Begin RollShop Widget
	UPROPERTY()
	TObjectPtr<URollShopWidgetController> RollShopWidgetController;

	// 새롭게 생성 시 필요한 StaticClass
	UPROPERTY(EditAnywhere)
	TSubclassOf<URollShopWidgetController> RollShopWidgetControllerClass;
	//~ End RollShop Widget
};
