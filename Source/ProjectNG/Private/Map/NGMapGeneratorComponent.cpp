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

	// 2. Ring Nodes (Layer 1 - Circular)
	int32 RingCount = RandomStream.RandRange(MinRingNodes, MaxRingNodes);
	for (int32 i = 0; i < RingCount; ++i)
	{
		FMapNodeData RingNode;
		RingNode.NodeID = NodeIDCounter++;
		RingNode.LayerIndex = 1;
		GeneratedNodes.Add(RingNode);
	}

	// 3. Branch Nodes (Layer 2 - Outward)
	int32 TotalNodes = RandomStream.RandRange(TotalNodeCountMin, TotalNodeCountMax);
	int32 RemainingNodes = TotalNodes - GeneratedNodes.Num() - NumberOfTowns;
	if (RemainingNodes < 0) RemainingNodes = 0;

	for (int32 i = 0; i < RemainingNodes; ++i)
	{
		FMapNodeData BranchNode;
		BranchNode.NodeID = NodeIDCounter++;
		BranchNode.LayerIndex = 2;
		GeneratedNodes.Add(BranchNode);
	}

	// 4. Town Nodes (Layer 0 - Start), attached later to outer branch nodes.
	for (int32 i = 0; i < NumberOfTowns; ++i)
	{
		FMapNodeData TownNode;
		TownNode.NodeID = NodeIDCounter++;
		TownNode.LayerIndex = 0;
		GeneratedNodes.Add(TownNode);
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

	// 3. Connect Branch Nodes
	TArray<FMapNodeData*> ActiveNodes = RingNodes; // Nodes that can sprout branches
	TMap<int32, int32> RootRingMap;
	for (FMapNodeData* RingNode : RingNodes)
	{
		RootRingMap.Add(RingNode->NodeID, RingNode->NodeID);
	}

	for (FMapNodeData* BranchNode : BranchNodes)
	{
		if (ActiveNodes.IsEmpty()) break;

		int32 SourceIdx = RandomStream.RandRange(0, ActiveNodes.Num() - 1);
		FMapNodeData* SourceNode = ActiveNodes[SourceIdx];

		BranchNode->ConnectedNodeIDs.AddUnique(SourceNode->NodeID);
		SourceNode->ConnectedNodeIDs.AddUnique(BranchNode->NodeID);

		if (RootRingMap.Contains(SourceNode->NodeID))
		{
			RootRingMap.Add(BranchNode->NodeID, RootRingMap[SourceNode->NodeID]);
		}

		ActiveNodes.Add(BranchNode);

		if (SourceNode->ConnectedNodeIDs.Num() >= MaxConnectionsPerNode)
		{
			ActiveNodes.RemoveAt(SourceIdx);
		}
	}

	// 4. Connect leaf nodes to each other occasionally (neighboring subtrees only to avoid cross-center lines)
	TArray<FMapNodeData*> LeafNodes;
	for (FMapNodeData* BranchNode : BranchNodes)
	{
		if (BranchNode->ConnectedNodeIDs.Num() == 1)
		{
			LeafNodes.Add(BranchNode);
		}
	}

	// Sort leaf nodes by their root Ring Node index to keep connections local
	LeafNodes.Sort([&RootRingMap, &RingNodes](const FMapNodeData& A, const FMapNodeData& B) {
		int32 RootA = RootRingMap.Contains(A.NodeID) ? RootRingMap[A.NodeID] : -1;
		int32 RootB = RootRingMap.Contains(B.NodeID) ? RootRingMap[B.NodeID] : -1;

		int32 IndexA = RingNodes.IndexOfByPredicate([RootA](const FMapNodeData* Node) { return Node->NodeID == RootA; });
		int32 IndexB = RingNodes.IndexOfByPredicate([RootB](const FMapNodeData* Node) { return Node->NodeID == RootB; });

		return IndexA < IndexB;
	});

	for (int32 i = 0; i < LeafNodes.Num() - 1; ++i)
	{
		if (RandomStream.FRand() < LeafConnectionProbability)
		{
			LeafNodes[i]->ConnectedNodeIDs.AddUnique(LeafNodes[i+1]->NodeID);
			LeafNodes[i+1]->ConnectedNodeIDs.AddUnique(LeafNodes[i]->NodeID);

			// Increment i to avoid chaining multiple connections to the same leaf
			i++;
		}
	}

	// 5. Attach Town nodes to outer branch leaves so town lines point outward.
	TArray<FMapNodeData*> TownParentCandidates;
	for (FMapNodeData* BranchNode : BranchNodes)
	{
		if (BranchNode->ConnectedNodeIDs.Num() <= 1)
		{
			TownParentCandidates.Add(BranchNode);
		}
	}

	if (TownParentCandidates.Num() < TownNodes.Num())
	{
		for (FMapNodeData* BranchNode : BranchNodes)
		{
			TownParentCandidates.AddUnique(BranchNode);
		}
	}

	if (TownParentCandidates.IsEmpty())
	{
		TownParentCandidates = RingNodes;
	}

	TownParentCandidates.Sort([&RootRingMap, &RingNodes](const FMapNodeData& A, const FMapNodeData& B) {
		int32 RootA = RootRingMap.Contains(A.NodeID) ? RootRingMap[A.NodeID] : A.NodeID;
		int32 RootB = RootRingMap.Contains(B.NodeID) ? RootRingMap[B.NodeID] : B.NodeID;

		int32 IndexA = RingNodes.IndexOfByPredicate([RootA](const FMapNodeData* Node) { return Node->NodeID == RootA; });
		int32 IndexB = RingNodes.IndexOfByPredicate([RootB](const FMapNodeData* Node) { return Node->NodeID == RootB; });

		return IndexA < IndexB;
	});

	for (int32 i = TownParentCandidates.Num() - 1; i > 0; --i)
	{
		int32 SwapIndex = RandomStream.RandRange(0, i);
		TownParentCandidates.Swap(i, SwapIndex);
	}

	for (int32 i = 0; i < TownNodes.Num(); ++i)
	{
		FMapNodeData* TownNode = TownNodes[i];
		FMapNodeData* ParentNode = TownParentCandidates[i % TownParentCandidates.Num()];

		TownNode->ConnectedNodeIDs.AddUnique(ParentNode->NodeID);
		ParentNode->ConnectedNodeIDs.AddUnique(TownNode->NodeID);
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
	TMap<int32, FMapNodeData*> NodeMap;
	for (FMapNodeData& Node : GeneratedNodes)
	{
		NodeMap.Add(Node.NodeID, &Node);
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

	TMap<int32, float> NodeAngles;
	TMap<int32, float> NodeRadii;

	for (int32 i = 0; i < RingNodes.Num(); ++i)
	{
		float Angle = (static_cast<float>(i) / RingNodes.Num()) * 2.0f * PI;
		float Radius = 0.24f;
		RingNodes[i]->Location = FVector(0.5f + FMath::Cos(Angle) * Radius, 0.5f + FMath::Sin(Angle) * Radius, 0.0f);
		NodeAngles.Add(RingNodes[i]->NodeID, Angle);
		NodeRadii.Add(RingNodes[i]->NodeID, Radius);
	}

	// BFS to position Branch nodes cleanly radiating outward
	TSet<int32> Visited;
	TArray<int32> Queue;

	for (FMapNodeData* RingNode : RingNodes)
	{
		Visited.Add(RingNode->NodeID);
		Queue.Add(RingNode->NodeID);
	}

	for (FMapNodeData& Node : GeneratedNodes)
	{
		if (Node.LayerIndex == 3)
		{
			Visited.Add(Node.NodeID);
		}
	}

	TMap<int32, int32> NodeDepth;
	for (FMapNodeData* RingNode : RingNodes)
	{
		NodeDepth.Add(RingNode->NodeID, 0);
	}

	while (Queue.Num() > 0)
	{
		int32 CurrentID = Queue[0];
		Queue.RemoveAt(0);

		FMapNodeData* CurrentNode = NodeMap[CurrentID];
		float ParentAngle = NodeAngles[CurrentID];
		float ParentRadius = NodeRadii[CurrentID];
		int32 CurrentDepth = NodeDepth[CurrentID];

		TArray<int32> Children;
		for (int32 NeighborID : CurrentNode->ConnectedNodeIDs)
		{
			if (NodeMap.Contains(NeighborID) && NodeMap[NeighborID]->LayerIndex == 2 && !Visited.Contains(NeighborID))
			{
				Children.Add(NeighborID);
			}
		}

		if (Children.Num() > 0)
		{
			float SpreadAngle = 0.55f / (CurrentDepth + 1);

			for (int32 i = 0; i < Children.Num(); ++i)
			{
				int32 ChildID = Children[i];
				FMapNodeData* ChildNode = NodeMap[ChildID];

				float Fraction = (Children.Num() > 1) ? (static_cast<float>(i) / (Children.Num() - 1)) - 0.5f : 0.0f;
				float AngleOffset = Fraction * SpreadAngle;

				float JitterAngle = RandomStream.FRandRange(-0.035f, 0.035f);
				float JitterRadius = RandomStream.FRandRange(-0.015f, 0.015f);

				float ChildAngle = ParentAngle + AngleOffset + JitterAngle;
				float ChildRadius = ParentRadius + 0.105f + JitterRadius;

				ChildRadius = FMath::Min(ChildRadius, 0.46f);

				ChildNode->Location = FVector(0.5f + FMath::Cos(ChildAngle) * ChildRadius, 0.5f + FMath::Sin(ChildAngle) * ChildRadius, 0.0f);

				NodeAngles.Add(ChildID, ChildAngle);
				NodeRadii.Add(ChildID, ChildRadius);
				NodeDepth.Add(ChildID, CurrentDepth + 1);

				Visited.Add(ChildID);
				Queue.Add(ChildID);
			}
		}
	}

	for (FMapNodeData* BranchNode : BranchNodes)
	{
		if (!NodeAngles.Contains(BranchNode->NodeID))
		{
			float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
			float Radius = RandomStream.FRandRange(0.28f, 0.42f);
			BranchNode->Location = FVector(0.5f + FMath::Cos(Angle) * Radius, 0.5f + FMath::Sin(Angle) * Radius, 0.0f);
			NodeAngles.Add(BranchNode->NodeID, Angle);
			NodeRadii.Add(BranchNode->NodeID, Radius);
		}
	}

	for (int32 i = 0; i < TownNodes.Num(); ++i)
	{
		FMapNodeData* TownNode = TownNodes[i];
		int32 ParentID = -1;
		for (int32 ConnectedID : TownNode->ConnectedNodeIDs)
		{
			if (NodeMap.Contains(ConnectedID))
			{
				ParentID = ConnectedID;
				break;
			}
		}

		float Angle = (static_cast<float>(i) / FMath::Max(1, TownNodes.Num())) * 2.0f * PI + (PI / FMath::Max(1, TownNodes.Num()));
		float Radius = 0.5f;
		if (ParentID != -1 && NodeAngles.Contains(ParentID) && NodeRadii.Contains(ParentID))
		{
			Angle = NodeAngles[ParentID] + RandomStream.FRandRange(-0.08f, 0.08f);
			Radius = FMath::Clamp(NodeRadii[ParentID] + 0.11f, 0.44f, 0.52f);
		}

		TownNode->Location = FVector(0.5f + FMath::Cos(Angle) * Radius, 0.5f + FMath::Sin(Angle) * Radius, 0.0f);
		NodeAngles.Add(TownNode->NodeID, Angle);
		NodeRadii.Add(TownNode->NodeID, Radius);
	}

	ResolveNodePositionOverlaps();
	AdjustLongConnectedNodeDistances();
	ResolveNodePositionOverlaps();
}

void UNGMapGeneratorComponent::ResolveNodePositionOverlaps()
{
	constexpr float MinDistance = 0.105f;
	constexpr float MinDistanceSq = MinDistance * MinDistance;
	constexpr float MinCoord = 0.07f;
	constexpr float MaxCoord = 0.93f;
	constexpr int32 IterationCount = 28;

	for (int32 Iteration = 0; Iteration < IterationCount; ++Iteration)
	{
		bool bMovedAny = false;

		for (int32 i = 0; i < GeneratedNodes.Num(); ++i)
		{
			for (int32 j = i + 1; j < GeneratedNodes.Num(); ++j)
			{
				FMapNodeData& A = GeneratedNodes[i];
				FMapNodeData& B = GeneratedNodes[j];

				if (A.LayerIndex == 3 || B.LayerIndex == 3)
				{
					continue;
				}

				FVector2D PosA(A.Location.X, A.Location.Y);
				FVector2D PosB(B.Location.X, B.Location.Y);
				FVector2D Delta = PosB - PosA;
				float DistanceSq = Delta.SizeSquared();

				if (DistanceSq >= MinDistanceSq)
				{
					continue;
				}

				if (DistanceSq <= KINDA_SMALL_NUMBER)
				{
					const float Angle = RandomStream.FRandRange(0.0f, 2.0f * PI);
					Delta = FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
					DistanceSq = 1.0f;
				}

				const float Distance = FMath::Sqrt(DistanceSq);
				const FVector2D Direction = Delta / Distance;
				const float PushAmount = (MinDistance - Distance) * 0.5f;

				PosA -= Direction * PushAmount;
				A.Location.X = FMath::Clamp(PosA.X, MinCoord, MaxCoord);
				A.Location.Y = FMath::Clamp(PosA.Y, MinCoord, MaxCoord);

				PosB += Direction * PushAmount;
				B.Location.X = FMath::Clamp(PosB.X, MinCoord, MaxCoord);
				B.Location.Y = FMath::Clamp(PosB.Y, MinCoord, MaxCoord);

				bMovedAny = true;
			}
		}

		if (!bMovedAny)
		{
			break;
		}
	}
}

void UNGMapGeneratorComponent::AdjustLongConnectedNodeDistances()
{
	constexpr float MaxEdgeDistance = 0.24f;
	constexpr float MaxEdgeDistanceSq = MaxEdgeDistance * MaxEdgeDistance;
	constexpr float MinCoord = 0.07f;
	constexpr float MaxCoord = 0.93f;
	constexpr int32 IterationCount = 10;

	auto GetMovePriority = [](const FMapNodeData& Node) -> int32
	{
		if (Node.LayerIndex == 0) return 3; // Town
		if (Node.LayerIndex == 2) return 2; // Branch
		if (Node.LayerIndex == 1) return 1; // Ring
		return 0;                           // Named
	};

	for (int32 Iteration = 0; Iteration < IterationCount; ++Iteration)
	{
		bool bMovedAny = false;

		for (FMapNodeData& Node : GeneratedNodes)
		{
			for (int32 ConnectedID : Node.ConnectedNodeIDs)
			{
				if (Node.NodeID >= ConnectedID)
				{
					continue;
				}

				FMapNodeData* ConnectedNode = GeneratedNodes.FindByPredicate([ConnectedID](const FMapNodeData& Candidate) {
					return Candidate.NodeID == ConnectedID;
				});

				if (!ConnectedNode)
				{
					continue;
				}

				FVector2D PosA(Node.Location.X, Node.Location.Y);
				FVector2D PosB(ConnectedNode->Location.X, ConnectedNode->Location.Y);
				FVector2D Delta = PosB - PosA;
				float DistanceSq = Delta.SizeSquared();

				if (DistanceSq <= MaxEdgeDistanceSq || DistanceSq <= KINDA_SMALL_NUMBER)
				{
					continue;
				}

				const float Distance = FMath::Sqrt(DistanceSq);
				const FVector2D Direction = Delta / Distance;
				const float PullAmount = (Distance - MaxEdgeDistance) * 0.65f;
				const int32 PriorityA = GetMovePriority(Node);
				const int32 PriorityB = GetMovePriority(*ConnectedNode);

				if (PriorityA == 0 && PriorityB == 0)
				{
					continue;
				}

				if (PriorityA > PriorityB)
				{
					PosA += Direction * PullAmount;
				}
				else if (PriorityB > PriorityA)
				{
					PosB -= Direction * PullAmount;
				}
				else
				{
					PosA += Direction * PullAmount * 0.5f;
					PosB -= Direction * PullAmount * 0.5f;
				}

				if (PriorityA > 0)
				{
					Node.Location.X = FMath::Clamp(PosA.X, MinCoord, MaxCoord);
					Node.Location.Y = FMath::Clamp(PosA.Y, MinCoord, MaxCoord);
				}

				if (PriorityB > 0)
				{
					ConnectedNode->Location.X = FMath::Clamp(PosB.X, MinCoord, MaxCoord);
					ConnectedNode->Location.Y = FMath::Clamp(PosB.Y, MinCoord, MaxCoord);
				}

				bMovedAny = true;
			}
		}

		if (!bMovedAny)
		{
			break;
		}
	}
}
