// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Map/MapNodeDataAsset.h"

TSubclassOf<ANGMapNode> UMapNodeDataAsset::GetMapNodeData(const FGameplayTag& Tag)
{
	for (FMapNodeParam& Data : MapNodeData)
	{
		if (Data.NodeTag == Tag)
		{
			return Data.MapNode;
		}
	}
	
	return nullptr;
}
