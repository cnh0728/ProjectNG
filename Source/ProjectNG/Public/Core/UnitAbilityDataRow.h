// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h"
#include "UnitAbilityDataRow.generated.h"

USTRUCT(BlueprintType)
struct FUnitSkillSlotData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", DisplayName = "슬롯 이름")
	FName SlotName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill", DisplayName = "기본 스킬")
	TSoftClassPtr<UGameplayAbility> DefaultSkillAbility;
};

USTRUCT(BlueprintType)
struct FUnitAbilityData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", DisplayName = "유닛 식별 태그")
	FGameplayTag IdentificationTag = FGameplayTag();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag", DisplayName = "유닛 보유 태그")
	FGameplayTagContainer OwnedTags = FGameplayTagContainer();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "기본 스탯 목록", meta=(ToolTip="MaxHealth, Health처럼 의존 관계가 있는 값은 최대값을 먼저 선언하세요. 예: MaxHealth=100, Health=100"))
	TMap<FGameplayAttribute, float> DefaultStats;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "고유 능력")
	TSoftClassPtr<UGameplayAbility> UniqueAbility;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "고유 패시브 능력")
	TSoftClassPtr<UGameplayAbility> PassiveAbility;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "스킬 슬롯 목록")
	TArray<FUnitSkillSlotData> SkillSlots;
};
