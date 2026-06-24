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
	
	FGameplayTag Node_Town_Elf;
	FGameplayTag Node_Town_Dwarf;
	FGameplayTag Node_Town_Human;

	// 노드 태그
	FGameplayTag Node_General;
	FGameplayTag Node_Shop;
	FGameplayTag Node_Combat;
	FGameplayTag Node_Event;
	FGameplayTag Node_Rest;
	FGameplayTag Node_Elite;
	FGameplayTag Node_Named;
	
	// 마을 버프 태그
	FGameplayTag TownBuff_ExtraGold;
	FGameplayTag TownBuff_AttackBoost;
	FGameplayTag TownBuff_DefenseBoost;
	FGameplayTag TownBuff_StartUnit;

private:
	static FNGGameplayTags GameplayTags;
};