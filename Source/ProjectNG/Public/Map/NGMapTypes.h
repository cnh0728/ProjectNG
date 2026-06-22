// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NGMapTypes.generated.h"

// 맵의 노드 데이터
USTRUCT(BlueprintType)
struct FMapNodeData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector Location = FVector(); // 월드 좌표
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FGameplayTag NodeTag = FGameplayTag();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 NodeID = -1; // 식별 ID
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<int32> ConnectedNodeIDs = TArray<int32>();
};