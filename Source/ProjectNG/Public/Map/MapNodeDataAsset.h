// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MapNodeDataAsset.generated.h"


struct FGameplayTag;
class ANGMapNode;

USTRUCT()
struct FMapNodeParam
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Node")
	FGameplayTag NodeTag = FGameplayTag();
	
	UPROPERTY(EditDefaultsOnly, Category = "Node")
	TSubclassOf<ANGMapNode> MapNode = nullptr;
};

/**
 * 
 */
UCLASS()
class PROJECTNG_API UMapNodeDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "MapNode")
	TArray<FMapNodeParam> MapNodeData;
	
	TSubclassOf<ANGMapNode> GetMapNodeData(const FGameplayTag& Tag);
};
