// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NGShopProbability.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FShopProbability : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Tier 1 확률", meta = (ToolTip="확률은 (0.0-1.0) 사이의 값입니다. 확률의 총합은 1.0을 넘을 수 없습니다."))
	float PercentOfTier1 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Tier 2 확률", meta = (ToolTip="확률은 (0.0-1.0) 사이의 값입니다. 확률의 총합은 1.0을 넘을 수 없습니다."))
	float PercentOfTier2 = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, DisplayName = "Tier 3 확률", meta = (ToolTip="확률은 (0.0-1.0) 사이의 값입니다. 확률의 총합은 1.0을 넘을 수 없습니다."))
	float PercentOfTier3 = 0.0f;
};
