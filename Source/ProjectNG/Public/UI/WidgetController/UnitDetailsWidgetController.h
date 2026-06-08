// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "UI/NGWidgetController.h"
#include "UnitDetailsWidgetController.generated.h"

class UNGUnitAttributeInfoDataAsset;
class ANGUnitPawn;
class UNGAbilitySystemComponent;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedSignature, FGameplayTag, AttributeTag, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitVisibilityChangedSignature, bool, bIsVisible);

UCLASS(BlueprintType, Blueprintable)
class PROJECTNG_API UUnitDetailsWidgetController : public UNGWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;
	
	void SetAttributeInfo(UNGUnitAttributeInfoDataAsset* NewAttributeInfo) { AttributeInfo = NewAttributeInfo; };
	
	void SetTargetUnit(ANGUnitPawn* NewUnit);
	void ClearTargetUnit();

protected:
	void OnAnyAttributeChanged(const struct FOnAttributeChangeData& Data);
	
public:
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnAttributeChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnUnitVisibilityChangedSignature OnUnitVisibilityChangedDelegate;
	
protected:
	UPROPERTY()
	TObjectPtr<UNGUnitAttributeInfoDataAsset> AttributeInfo;
	
	TWeakObjectPtr<ANGUnitPawn> TargetUnit;
	TMap<FGameplayAttribute, FDelegateHandle> AttributeDelegateHandles;
	
};
