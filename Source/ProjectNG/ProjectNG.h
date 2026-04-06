// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define ECC_Map ECC_GameTraceChannel1
#define ECC_SelectableUnit ECC_GameTraceChannel2
#define ECC_Enemy ECC_GameTraceChannel3

enum class EAbilityInputID : uint8
{
	None        UMETA(DisplayName = "None"),
    
	Confirm     UMETA(DisplayName = "Confirm"),
	Cancel      UMETA(DisplayName = "Cancel"),
    
	Attack      UMETA(DisplayName = "Attack"), // 마우스 왼쪽 클릭 등
	Skill       UMETA(DisplayName = "Skill"),  // Q나 E 키 등
	Jump        UMETA(DisplayName = "Jump")    // 스페이스바 등
};