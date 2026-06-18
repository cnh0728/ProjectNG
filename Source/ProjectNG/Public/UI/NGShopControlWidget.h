// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGUserWidget.h"
#include "NGShopControlWidget.generated.h"

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
	
	UFUNCTION()
	void OnGoldChanged(float NewGold);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FleeButton;
};
