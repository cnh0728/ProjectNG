// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/NGWidgetController.h"
#include "NGRollShopWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChangedSignature, float, NewGold);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTNG_API UNGRollShopWidgetController : public UNGWidgetController
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
	
	UPROPERTY(BlueprintAssignable, Category = "Game|Shop")
	FOnGoldChangedSignature OnGoldChanged;
};
