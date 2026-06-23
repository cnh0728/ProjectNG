// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Map/NGMapGenerator.h"
#include "Core/NGGameplayTags.h"
#include "Map/MapNodeDataAsset.h"

ANGMapGenerator::ANGMapGenerator()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent->SetMobility(EComponentMobility::Static);
}

void ANGMapGenerator::GenerateMap(int32 Seed)
{
	RandomSeed = Seed;
	RandomStream.Initialize(RandomSeed);
	
	UE_LOG(LogTemp, Warning, TEXT("Map Seed: %d"), RandomSeed);
	
	int32 MaxRetries = 3;
	bool bSuccess = false;
	
	for (int32 i = 0; i < MaxRetries; ++i)
	{
		GeneratedNodes.Empty();
		
		GenerateLayerNodes();
		ConnectLayers();
		
		if (ValidateConnectivity())
		{
			bSuccess = true;
			break;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Map generation validation failed. Retrying... (%d/%d)"), i + 1, MaxRetries);
		}
	}
	
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to generate a valid map after %d retries."), MaxRetries);
		return;
	}
	
	AssignNodeTypes();
	AssignTownBuffs();
	AssignNodePositions();
}

void ANGMapGenerator::BeginPlay()
{
	Super::BeginPlay();
}

FGameplayTag ANGMapGenerator::GetGameplayTagById(int32 NodeID)
{
	for (const FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.NodeID == NodeID)
		{
			return Node.NodeTag;
		}
	}
	return FGameplayTag();
}

TArray<const FMapNodeData*> ANGMapGenerator::GetNodesAtLayer(int32 LayerIndex) const
{
	TArray<const FMapNodeData*> Result;
	for (const FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == LayerIndex)
		{
			Result.Add(&Node);
		}
	}
	return Result;
}

TArray<const FMapNodeData*> ANGMapGenerator::GetTownNodes() const
{
	return GetNodesAtLayer(0);
}

int32 ANGMapGenerator::CalculateNodesForLayer(int32 LayerIndex)
{
	if (LayerIndex == 0) return NumberOfTowns;
	
	if (BottleneckLayers.Contains(LayerIndex))
	{
		return BottleneckNodeCount;
	}
	
	if (LayerIndex == NumberOfLayers - 1)
	{
		return FMath::Min(MinNodesPerLayer, 3);
	}
	
	float Alpha = static_cast<float>(LayerIndex) / static_cast<float>(NumberOfLayers - 1);
	int32 BaseCount = FMath::RoundToInt(FMath::Lerp(static_cast<float>(MaxNodesPerLayer), static_cast<float>(MinNodesPerLayer), Alpha));
	
	int32 Offset = RandomStream.RandRange(-1, 1);
	int32 Count = BaseCount + Offset;
	
	Count = FMath::Clamp(Count, MinNodesPerLayer, MaxNodesPerLayer);
	
	return Count;
}

void ANGMapGenerator::GenerateLayerNodes()
{
	int32 NodeIDCounter = 0;
	
	for (int32 Layer = 0; Layer < NumberOfLayers; ++Layer)
	{
		int32 NodeCount = CalculateNodesForLayer(Layer);
		
		for (int32 i = 0; i < NodeCount; ++i)
		{
			FMapNodeData NewNode;
			NewNode.NodeID = NodeIDCounter++;
			NewNode.LayerIndex = Layer;
			GeneratedNodes.Add(NewNode);
		}
	}
}

void ANGMapGenerator::ConnectLayers()
{
	for (int32 Layer = 0; Layer < NumberOfLayers - 1; ++Layer)
	{
		TArray<FMapNodeData*> CurrentLayerNodes;
		TArray<FMapNodeData*> NextLayerNodes;
		
		for (FMapNodeData& Node : GeneratedNodes)
		{
			if (Node.LayerIndex == Layer) CurrentLayerNodes.Add(&Node);
			else if (Node.LayerIndex == Layer + 1) NextLayerNodes.Add(&Node);
		}
		
		if (CurrentLayerNodes.IsEmpty() || NextLayerNodes.IsEmpty()) continue;
		
		// 1. Give each current node some forward connections
		for (FMapNodeData* CurrentNode : CurrentLayerNodes)
		{
			int32 NumConnections = RandomStream.RandRange(MinConnectionsPerNode, MaxConnectionsPerNode);
			NumConnections = FMath::Min(NumConnections, NextLayerNodes.Num());
			
			TArray<FMapNodeData*> AvailableTargets = NextLayerNodes;
			
			for (int32 i = 0; i < NumConnections; ++i)
			{
				int32 TargetIdx = RandomStream.RandRange(0, AvailableTargets.Num() - 1);
				CurrentNode->ConnectedNodeIDs.AddUnique(AvailableTargets[TargetIdx]->NodeID);
				AvailableTargets.RemoveAt(TargetIdx);
			}
		}
		
		// 2. Ensure every next layer node has at least one incoming connection
		for (FMapNodeData* NextNode : NextLayerNodes)
		{
			bool bHasIncoming = false;
			for (FMapNodeData* CurrentNode : CurrentLayerNodes)
			{
				if (CurrentNode->ConnectedNodeIDs.Contains(NextNode->NodeID))
				{
					bHasIncoming = true;
					break;
				}
			}
			
			if (!bHasIncoming)
			{
				int32 SourceIdx = RandomStream.RandRange(0, CurrentLayerNodes.Num() - 1);
				CurrentLayerNodes[SourceIdx]->ConnectedNodeIDs.AddUnique(NextNode->NodeID);
			}
		}
	}
}

bool ANGMapGenerator::ValidateConnectivity()
{
	TArray<const FMapNodeData*> TownNodes = GetTownNodes();
	if (TownNodes.IsEmpty()) return false;
	
	for (const FMapNodeData* TownNode : TownNodes)
	{
		TArray<int32> Queue;
		TSet<int32> Visited;
		
		Queue.Add(TownNode->NodeID);
		Visited.Add(TownNode->NodeID);
		
		bool bReachedEnd = false;
		
		while (Queue.Num() > 0)
		{
			int32 CurrentID = Queue[0];
			Queue.RemoveAt(0);
			
			const FMapNodeData* CurrentNode = nullptr;
			for (const FMapNodeData& Node : GeneratedNodes)
			{
				if (Node.NodeID == CurrentID)
				{
					CurrentNode = &Node;
					break;
				}
			}
			
			if (!CurrentNode) continue;
			
			if (CurrentNode->LayerIndex == NumberOfLayers - 1)
			{
				bReachedEnd = true;
				break; // Optimization: early exit for this town if it reaches the end
			}
			
			for (int32 ConnectedID : CurrentNode->ConnectedNodeIDs)
			{
				if (!Visited.Contains(ConnectedID))
				{
					Visited.Add(ConnectedID);
					Queue.Add(ConnectedID);
				}
			}
		}
		
		if (!bReachedEnd)
		{
			return false; // This town cannot reach the end
		}
	}
	
	return true;
}

ENodeType ANGMapGenerator::PickRandomNodeType(int32 LayerIndex)
{
	float EWeight = EmptyWeight;
	float SWeight = ShopWeight;
	float CWeight = CombatWeight;
	float EvWeight = EventWeight;
	float RWeight = RestWeight;
	float ElWeight = EliteWeight;
	
	if (LayerIndex <= 3)
	{
		EWeight *= 1.5f;
		SWeight *= 1.5f;
		CWeight *= 0.8f;
		ElWeight *= 0.0f;
	}
	else if (LayerIndex <= 6)
	{
		CWeight *= 1.5f;
		EvWeight *= 1.5f;
		ElWeight *= 0.5f;
	}
	else if (LayerIndex <= 9)
	{
		CWeight *= 2.0f;
		ElWeight *= 2.0f;
		EWeight *= 0.3f;
		RWeight *= 0.5f;
	}
	
	float TotalWeight = EWeight + SWeight + CWeight + EvWeight + RWeight + ElWeight;
	if (TotalWeight <= 0.0f) return ENodeType::Empty;
	
	float RandVal = RandomStream.FRandRange(0.0f, TotalWeight);
	
	if (RandVal < EWeight) return ENodeType::Empty;
	RandVal -= EWeight;
	if (RandVal < SWeight) return ENodeType::Shop;
	RandVal -= SWeight;
	if (RandVal < CWeight) return ENodeType::Combat;
	RandVal -= CWeight;
	if (RandVal < EvWeight) return ENodeType::Event;
	RandVal -= EvWeight;
	if (RandVal < RWeight) return ENodeType::Rest;
	
	return ENodeType::Elite;
}

FGameplayTag ANGMapGenerator::GetTagForNodeType(ENodeType Type)
{
	const FNGGameplayTags& GameplayTags = FNGGameplayTags::Get();
	switch (Type)
	{
		case ENodeType::Town:
			return (RandomStream.FRand() < 0.5f) ? GameplayTags.Node_Town_Elf : GameplayTags.Node_Town_Dwarf;
		case ENodeType::Empty:   return GameplayTags.Node_Empty;
		case ENodeType::Shop:    return GameplayTags.Node_Shop;
		case ENodeType::Combat:  return GameplayTags.Node_Event_Combat;
		case ENodeType::Event:   return GameplayTags.Node_Event_Default;
		case ENodeType::Rest:    return GameplayTags.Node_Rest;
		case ENodeType::Elite:   return GameplayTags.Node_Elite;
		default:                 return FGameplayTag();
	}
}

void ANGMapGenerator::AssignNodeTypes()
{
	for (FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == 0)
		{
			Node.NodeType = ENodeType::Town;
		}
		else if (Node.LayerIndex == NumberOfLayers - 1)
		{
			Node.NodeType = (RandomStream.FRand() < 0.5f) ? ENodeType::Combat : ENodeType::Event;
		}
		else
		{
			Node.NodeType = PickRandomNodeType(Node.LayerIndex);
		}
		
		Node.NodeTag = GetTagForNodeType(Node.NodeType);
	}
}

void ANGMapGenerator::AssignTownBuffs()
{
	const FNGGameplayTags& GameplayTags = FNGGameplayTags::Get();
	TArray<FGameplayTag> AvailableBuffs = {
		GameplayTags.TownBuff_ExtraGold,
		GameplayTags.TownBuff_AttackBoost,
		GameplayTags.TownBuff_DefenseBoost,
		GameplayTags.TownBuff_StartUnit
	};
	
	// Shuffle buffs
	for (int32 i = AvailableBuffs.Num() - 1; i > 0; --i)
	{
		int32 j = RandomStream.RandRange(0, i);
		AvailableBuffs.Swap(i, j);
	}
	
	int32 BuffIndex = 0;
	for (FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == 0 && Node.NodeType == ENodeType::Town)
		{
			if (AvailableBuffs.IsValidIndex(BuffIndex))
			{
				Node.TownBuffTag = AvailableBuffs[BuffIndex++];
			}
		}
	}
}

void ANGMapGenerator::AssignNodePositions()
{
	// Logical coordinates for 2D UI mapping
	for (int32 Layer = 0; Layer < NumberOfLayers; ++Layer)
	{
		TArray<FMapNodeData*> LayerNodes;
		for (FMapNodeData& Node : GeneratedNodes)
		{
			if (Node.LayerIndex == Layer) LayerNodes.Add(&Node);
		}
		
		int32 NumNodes = LayerNodes.Num();
		if (NumNodes == 0) continue;
		
		for (int32 i = 0; i < NumNodes; ++i)
		{
			// X: evenly distributed between 0.1 and 0.9 + jitter
			float BaseX = (NumNodes == 1) ? 0.5f : 0.1f + 0.8f * (static_cast<float>(i) / (NumNodes - 1));
			float JitterX = RandomStream.FRandRange(-0.05f, 0.05f);
			if (NumNodes == 1) JitterX = 0.0f; // No jitter for single node
			
			float X = FMath::Clamp(BaseX + JitterX, 0.0f, 1.0f);
			float Y = static_cast<float>(Layer); // Let UI scale the Y axis
			
			LayerNodes[i]->Location = FVector(X, Y, 0.0f);
		}
	}
}


