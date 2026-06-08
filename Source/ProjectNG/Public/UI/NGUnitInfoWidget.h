// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGUserWidget.h"
#include "Pawn/NGPawnBase.h"
#include "NGUnitInfoWidget.generated.h"

struct FOnAttributeChangeData;
class ANGUnitPawn;
class UNGAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PROJECTNG_API UNGUnitInfoWidget : public UNGUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetTargetUnit(ANGPawnBase* NewUnit);
	void ClearTargetUnit();

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateUnitInfo(float CurrentHP, float MaxHP, float Strength);
	
private:
	TWeakObjectPtr<UNGAbilitySystemComponent> TargetASC;
	
	FDelegateHandle HealthChangedDelegateHandle;
	FDelegateHandle StrengthChangedDelegateHandle;
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnStrengthChanged(const FOnAttributeChangeData& Data);
};
