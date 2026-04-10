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
	FGameplayTag IdentificationTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag", DisplayName = "유닛 부여 태그")
	FGameplayTagContainer OwnedTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", DisplayName = "UI 출력 이름")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "체력")
	float Health;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "공격력")
	float AttackDamage; // AD
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "주문력")
	float AbilityPower; // AP
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "공격 사거리")
	float AttackRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "공격 속도")
	float AttackSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "크리티컬 확률")
	float CriticalRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "회피율")
	float DodgeRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "물리 방어력")
	float PhysicalDefense;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "마법 저항력")
	float MagicDefense;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "고유 스킬")
	TSoftObjectPtr<UGameplayAbility> UniqueAbility;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "금전 스킬")
	TSoftObjectPtr<UGameplayAbility> GoldAbility;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability", DisplayName = "범용 스킬 슬롯")
	int32 MaxSkillSlots = 1;
};