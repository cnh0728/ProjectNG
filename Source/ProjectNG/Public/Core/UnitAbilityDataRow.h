// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "UnitAbilityDataRow.generated.h"

USTRUCT(BlueprintType)
struct FUnitAbilityData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag", DisplayName = "유닛 식별 태그")
	FGameplayTag IdentificationTag = FGameplayTag();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag", DisplayName = "유닛 부여 태그")
	FGameplayTagContainer OwnedTags = FGameplayTagContainer();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", DisplayName = "UI 출력 이름")
	FText DisplayName = FText();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "유닛 기본 스탯 목록", meta=(ToolTip="최대 체력이나 마나가 먼저 선언되어야 합니다. 예: MaxHealth=100, Health=100"))
	TMap<FGameplayAttribute, float> DefaultStats;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "고유 스킬")
	TSoftObjectPtr<UGameplayAbility> UniqueAbility = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "금전 스킬")
	TSoftObjectPtr<UGameplayAbility> GoldAbility = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "범용 스킬 슬롯")
	int32 MaxSkillSlots = 1;
};