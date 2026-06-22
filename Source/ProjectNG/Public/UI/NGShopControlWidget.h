// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGUserWidget.h"
#include "NGShopControlWidget.generated.h"

class UTextBlock;
class UNGWidgetController;
class UButton;
/**
 * 
 */
UCLASS()
class PROJECTNG_API UNGShopControlWidget : public UNGUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnFleeButtonClicked();
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateGoldText(float NewGold);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FleeButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;
};
