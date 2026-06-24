// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Map/NGMapGeneratorComponent.h"
#include "Core/NGGameplayTags.h"
#include "Map/MapNodeDataAsset.h"

UNGMapGeneratorComponent::UNGMapGeneratorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNGMapGeneratorComponent::GenerateMap(int32 Seed)
{
	RandomSeed = Seed;
	RandomStream.Initialize(RandomSeed);
	
	UE_LOG(LogTemp, Warning, TEXT("Map Seed: %d"), RandomSeed);
	
	int32 MaxRetries = 3;
	bool bSuccess = false;
	
	for (int32 i = 0; i < MaxRetries; ++i)
	{
		GeneratedNodes.Empty();
		
		GenerateNodes();
		ConnectNodes();
		
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

void UNGMapGeneratorComponent::BeginPlay()
{
	Super::BeginPlay();
}

FGameplayTag UNGMapGeneratorComponent::GetGameplayTagById(int32 NodeID)
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

TArray<const FMapNodeData*> UNGMapGeneratorComponent::GetNodesAtLayer(int32 LayerIndex) const
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

TArray<const FMapNodeData*> UNGMapGeneratorComponent::GetTownNodes() const
{
	return GetNodesAtLayer(0);
}

void UNGMapGeneratorComponent::GenerateNodes()
{
	int32 NodeIDCounter = 0;
	
	// 1. Named Node (Layer 3 - Center)
	FMapNodeData NamedNode;
	NamedNode.NodeID = NodeIDCounter++;
	NamedNode.LayerIndex = 3;
	GeneratedNodes.Add(NamedNode);
	
	// 2. Town Nodes (Layer 0 - Start)
	for (int32 i = 0; i < NumberOfTowns; ++i)
	{
		FMapNodeData TownNode;
		TownNode.NodeID = NodeIDCounter++;
		TownNode.LayerIndex = 0;
		GeneratedNodes.Add(TownNode);
	}
	
	// 3. Ring Nodes (Layer 1 - Circular)
	int32 RingCount = RandomStream.RandRange(MinRingNodes, MaxRingNodes);
	for (int32 i = 0; i < RingCount; ++i)
	{
		FMapNodeData RingNode;
		RingNode.NodeID = NodeIDCounter++;
		RingNode.LayerIndex = 1;
		GeneratedNodes.Add(RingNode);
	}
	
	// 4. Branch Nodes (Layer 2 - Outward)
	int32 TotalNodes = RandomStream.RandRange(TotalNodeCountMin, TotalNodeCountMax);
	int32 RemainingNodes = TotalNodes - GeneratedNodes.Num();
	if (RemainingNodes < 0) RemainingNodes = 0;
	
	for (int32 i = 0; i < RemainingNodes; ++i)
	{
		FMapNodeData BranchNode;
		BranchNode.NodeID = NodeIDCounter++;
		BranchNode.LayerIndex = 2;
		GeneratedNodes.Add(BranchNode);
	}
}

void UNGMapGeneratorComponent::ConnectNodes()
{
	TArray<FMapNodeData*> NamedNodes, TownNodes, RingNodes, BranchNodes;
	for (FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == 0) TownNodes.Add(&Node);
		else if (Node.LayerIndex == 1) RingNodes.Add(&Node);
		else if (Node.LayerIndex == 2) BranchNodes.Add(&Node);
		else if (Node.LayerIndex == 3) NamedNodes.Add(&Node);
	}
	
	if (NamedNodes.IsEmpty() || RingNodes.IsEmpty()) return;
	
	FMapNodeData* NamedNode = NamedNodes[0];
	
	// 1. Connect Named to all Ring Nodes
	for (FMapNodeData* RingNode : RingNodes)
	{
		NamedNode->ConnectedNodeIDs.AddUnique(RingNode->NodeID);
		RingNode->ConnectedNodeIDs.AddUnique(NamedNode->NodeID);
	}
	
	// 2. Connect Ring Nodes in a loop
	for (int32 i = 0; i < RingNodes.Num(); ++i)
	{
		FMapNodeData* Current = RingNodes[i];
		FMapNodeData* Next = RingNodes[(i + 1) % RingNodes.Num()];
		Current->ConnectedNodeIDs.AddUnique(Next->NodeID);
		Next->ConnectedNodeIDs.AddUnique(Current->NodeID);
	}
	
	// 3. Connect each Town to a distinct Ring Node
	TArray<FMapNodeData*> AvailableRingNodes = RingNodes;
	for (FMapNodeData* TownNode : TownNodes)
	{
		if (AvailableRingNodes.IsEmpty()) AvailableRingNodes = RingNodes; // Fallback
		
		int32 TargetIdx = RandomStream.RandRange(0, AvailableRingNodes.Num() - 1);
		FMapNodeData* TargetRing = AvailableRingNodes[TargetIdx];
		
		TownNode->ConnectedNodeIDs.AddUnique(TargetRing->NodeID);
		TargetRing->ConnectedNodeIDs.AddUnique(TownNode->NodeID);
		
		AvailableRingNodes.RemoveAt(TargetIdx);
	}
	
	// 4. Connect Branch Nodes
	TArray<FMapNodeData*> ActiveNodes = RingNodes; // Nodes that can sprout branches
	for (FMapNodeData* BranchNode : BranchNodes)
	{
		if (ActiveNodes.IsEmpty()) break;
		
		int32 SourceIdx = RandomStream.RandRange(0, ActiveNodes.Num() - 1);
		FMapNodeData* SourceNode = ActiveNodes[SourceIdx];
		
		BranchNode->ConnectedNodeIDs.AddUnique(SourceNode->NodeID);
		SourceNode->ConnectedNodeIDs.AddUnique(BranchNode->NodeID);
		
		ActiveNodes.Add(BranchNode);
		
		if (SourceNode->ConnectedNodeIDs.Num() >= MaxConnectionsPerNode)
		{
			ActiveNodes.RemoveAt(SourceIdx);
		}
	}
	
	// 5. Connect leaf nodes to each other occasionally
	TArray<FMapNodeData*> LeafNodes;
	for (FMapNodeData* BranchNode : BranchNodes)
	{
		if (BranchNode->ConnectedNodeIDs.Num() == 1)
		{
			LeafNodes.Add(BranchNode);
		}
	}
	
	for (int32 i = LeafNodes.Num() - 1; i > 0; --i)
	{
		int32 j = RandomStream.RandRange(0, i);
		LeafNodes.Swap(i, j);
	}
	
	for (int32 i = 0; i < LeafNodes.Num(); i += 2)
	{
		if (i + 1 < LeafNodes.Num())
		{
			if (RandomStream.FRand() < LeafConnectionProbability)
			{
				LeafNodes[i]->ConnectedNodeIDs.AddUnique(LeafNodes[i+1]->NodeID);
				LeafNodes[i+1]->ConnectedNodeIDs.AddUnique(LeafNodes[i]->NodeID);
			}
		}
	}
}

bool UNGMapGeneratorComponent::ValidateConnectivity()
{
	if (GeneratedNodes.IsEmpty()) return false;
	
	TArray<int32> Queue;
	TSet<int32> Visited;
	
	Queue.Add(GeneratedNodes[0].NodeID);
	Visited.Add(GeneratedNodes[0].NodeID);
	
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
		
		for (int32 ConnectedID : CurrentNode->ConnectedNodeIDs)
		{
			if (!Visited.Contains(ConnectedID))
			{
				Visited.Add(ConnectedID);
				Queue.Add(ConnectedID);
			}
		}
	}
	
	return Visited.Num() == GeneratedNodes.Num();
}

ENodeType UNGMapGeneratorComponent::PickRandomNodeType()
{
	float GWeight = GeneralWeight;
	float SWeight = ShopWeight;
	float CWeight = CombatWeight;
	float EvWeight = EventWeight;
	float RWeight = RestWeight;
	float ElWeight = EliteWeight;
	
	float TotalWeight = GWeight + SWeight + CWeight + EvWeight + RWeight + ElWeight;
	if (TotalWeight <= 0.0f) return ENodeType::General;
	
	float RandVal = RandomStream.FRandRange(0.0f, TotalWeight);
	
	if (RandVal < GWeight) return ENodeType::General;
	RandVal -= GWeight;
	if (RandVal < SWeight) return ENodeType::Shop;
	RandVal -= SWeight;
	if (RandVal < CWeight) return ENodeType::Combat;
	RandVal -= CWeight;
	if (RandVal < EvWeight) return ENodeType::Event;
	RandVal -= EvWeight;
	if (RandVal < RWeight) return ENodeType::Rest;
	
	return ENodeType::Elite;
}

FGameplayTag UNGMapGeneratorComponent::GetTagForNodeType(ENodeType Type)
{
	const FNGGameplayTags& GameplayTags = FNGGameplayTags::Get();
	switch (Type)
	{
		case ENodeType::Town:
			return (RandomStream.FRand() < 0.5f) ? GameplayTags.Node_Town_Elf : GameplayTags.Node_Town_Dwarf;
		case ENodeType::General: return GameplayTags.Node_General;
		case ENodeType::Shop:    return GameplayTags.Node_Shop;
		case ENodeType::Combat:  return GameplayTags.Node_Combat;
		case ENodeType::Event:   return GameplayTags.Node_Event;
		case ENodeType::Rest:    return GameplayTags.Node_Rest;
		case ENodeType::Elite:   return GameplayTags.Node_Elite;
		case ENodeType::Named:   return GameplayTags.Node_Named;
		default:                 return FGameplayTag();
	}
}

void UNGMapGeneratorComponent::AssignNodeTypes()
{
	for (FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == 0)
		{
			Node.NodeType = ENodeType::Town;
		}
		else if (Node.LayerIndex == 3)
		{
			Node.NodeType = ENodeType::Named;
		}
		else
		{
			Node.NodeType = PickRandomNodeType();
		}
		
		Node.NodeTag = GetTagForNodeType(Node.NodeType);
	}
}

void UNGMapGeneratorComponent::AssignTownBuffs()
{
	const FNGGameplayTags& GameplayTags = FNGGameplayTags::Get();
	TArray<FGameplayTag> AvailableBuffs = {
		GameplayTags.TownBuff_ExtraGold,
		GameplayTags.TownBuff_AttackBoost,
		GameplayTags.TownBuff_DefenseBoost,
		GameplayTags.TownBuff_StartUnit
	};
	
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

void UNGMapGeneratorComponent::AssignNodePositions()
{
	for (FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == 3) // Named
		{
			Node.Location = FVector(0.5f, 0.5f, 0.0f);
		}
	}
	
	TArray<FMapNodeData*> RingNodes, BranchNodes, TownNodes;
	for (FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == 1) RingNodes.Add(&Node);
		else if (Node.LayerIndex == 2) BranchNodes.Add(&Node);
		else if (Node.LayerIndex == 0) TownNodes.Add(&Node);
	}
	
	for (int32 i = 0; i < RingNodes.Num(); ++i)
	{
		float Angle = (static_cast<float>(i) / RingNodes.Num()) * 2.0f * PI;
		float Radius = 0.2f;
		RingNodes[i]->Location = FVector(0.5f + FMath::Cos(Angle) * Radius, 0.5f + FMath::Sin(Angle) * Radius, 0.0f);
	}
	
	for (int32 i = 0; i < TownNodes.Num(); ++i)
	{
		float Angle = (static_cast<float>(i) / TownNodes.Num()) * 2.0f * PI + (PI / TownNodes.Num());
		float Radius = 0.45f;
		TownNodes[i]->Location = FVector(0.5f + FMath::Cos(Angle) * Radius, 0.5f + FMath::Sin(Angle) * Radius, 0.0f);
	}
	
	for (int32 i = 0; i < BranchNodes.Num(); ++i)
	{
		float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
		float Radius = RandomStream.FRandRange(0.25f, 0.35f);
		BranchNodes[i]->Location = FVector(0.5f + FMath::Cos(Angle) * Radius, 0.5f + FMath::Sin(Angle) * Radius, 0.0f);
	}
}
