#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "EnemyAbilityData.generated.h"

USTRUCT(BlueprintType)
struct FEnemyAbilityData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", DisplayName = "유닛 식별 태그")
	FGameplayTag IdentificationTag = FGameplayTag();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat", DisplayName = "기본 스탯 목록", meta=(ToolTip="MaxHealth, Health처럼 의존 관계가 있는 값은 최대값을 먼저 선언하세요. 예: MaxHealth=100, Health=100"))
	TMap<FGameplayAttribute, float> DefaultStats;
};
