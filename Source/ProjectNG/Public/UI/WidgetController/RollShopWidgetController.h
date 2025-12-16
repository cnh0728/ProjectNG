// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/NGWidgetController.h"
#include "RollShopWidgetController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API URollShopWidgetController : public UNGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable, Category = "Game|Shop")
	void GetPlayerRollPocket(TArray<FName>& RollPockets) const;

	UFUNCTION(BlueprintCallable, Category = "Game|Shop")
	void RequestPlayerRollPocket() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Shop")
	int32 GainPlayerLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Game|Shop")
	void BuyUnitFromPocket(FName UnitName);
};
