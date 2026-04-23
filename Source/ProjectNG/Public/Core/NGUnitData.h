// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Pawn/NGUnitPawn.h"
#include "NGUnitData.generated.h"

UENUM(BlueprintType)
enum class EUnitTier : uint8
{
	Tier1,
	Tier2,
	Tier3
};

/**
 * 
 */
USTRUCT(BlueprintType)
struct FUnitData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "유닛 식별 아이디")
	int32 UnitID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "유닛 이름")
	FText UnitName = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "유닛 티어")
	EUnitTier Tier = EUnitTier::Tier1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "생성 할 유닛 액터 클래스")
	TSubclassOf<ANGUnitPawn> UnitClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "유닛 이미지")
	TSoftObjectPtr<UTexture2D> UnitTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "유닛 총 개수")
	int32  TotalCountInPool = -1;
};
