// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGEnum.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	None,
	PVECombat,
	PVPCombat,
	Village,
	Shop,
};
