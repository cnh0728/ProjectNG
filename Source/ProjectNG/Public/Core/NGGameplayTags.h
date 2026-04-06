// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "NGGameplayTags.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FNGGameplayTags
{
	GENERATED_BODY()
public:
	static const FNGGameplayTags& Get() {return GameplayTags;}
	
	static void InitializeNativeTags();
	
	FGameplayTag Event_Attack_Hit;

private:
	static FNGGameplayTags GameplayTags;
};