// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGAttributeSet.h"
#include "NGPlayerAttributeSet.generated.h"

UCLASS()
class PROJECTNG_API UNGPlayerAttributeSet : public UNGAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
    
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Gold)
	FGameplayAttributeData Gold;
	ATTRIBUTE_ACCESSORS(UNGPlayerAttributeSet, Gold)
	UFUNCTION()
	void OnRep_Gold(const FGameplayAttributeData& OldValue);
	
};
