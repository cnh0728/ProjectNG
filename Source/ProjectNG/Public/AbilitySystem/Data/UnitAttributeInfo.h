// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AttributeSet.h" // FGameplayAttribute를 사용하기 위해 필요
#include "UnitAttributeInfo.generated.h"

/**
 * UI에 표시할 단일 어트리뷰트(스탯)의 정보를 담는 구조체입니다.
 */
USTRUCT(BlueprintType)
struct FUnitAttributeInfo
{
	GENERATED_BODY()

	// 이 어트리뷰트를 식별할 고유 태그 (예: Attribute.Health)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	FGameplayTag AttributeTag;

	// 실제 시스템과 연동될 GAS 어트리뷰트 (예: UNGAttributeSet::GetHealthAttribute)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	FGameplayAttribute Attribute;

	// UI에 표시될 이름 (예: "체력", "공격력")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	FText AttributeName;

	// UI 툴팁 등에 사용될 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	FText AttributeDescription;

	// 값 브로드캐스트 시 담아서 보낼 임시 변수 (초기값 0)
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	float AttributeValue = 0.f;
};

/**
 * 여러 어트리뷰트 정보(FNGAttributeInfo)들을 배열로 관리하는 데이터 에셋입니다.
 */
UCLASS()
class PROJECTNG_API UNGUnitAttributeInfoDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 태그를 기반으로 어트리뷰트 정보를 찾아 반환합니다. */
	FUnitAttributeInfo FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound = false) const;

	// 에디터에서 세팅할 어트리뷰트 정보 배열
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AttributeInfo")
	TArray<FUnitAttributeInfo> AttributeInformation;
};
