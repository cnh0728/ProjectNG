// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "GameModes/NGInGameMode.h"

#include "Components/NGCombatManagerComponent.h"
#include "Core/NGDeveloperSettings.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Game/NGPawnDataManager.h"
#include "Map/NGMapGeneratorComponent.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerState.h"


ANGInGameMode::ANGInGameMode()
{
	CombatManagerComponent = CreateDefaultSubobject<UNGCombatManagerComponent>(TEXT("CombatManager"));
	MapGeneratorComponent = CreateDefaultSubobject<UNGMapGeneratorComponent>(TEXT("MapGenerator"));
}

void ANGInGameMode::RequestStartCombat(APlayerController* PC, bool bIsCPUCombat)
{
	// TODO: 테스트용으로 잠시 막음
	// if (CurrentState == EGameState::Combat)	return;
	
	ANGGameState* GS = GetGameState<ANGGameState>();
		
	// 테스트용
	if (GS)
	{
		for (APlayerState* RawPS : GS->PlayerArray)
		{
			if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
			{
				if (bIsCPUCombat)
				{
					TSoftObjectPtr<UNGEnemyDataAsset> SoftPath = GetDefault<UNGDeveloperSettings>()->EnemyDataAsset;

					if (SoftPath.IsNull()) return;

					UNGEnemyDataAsset* LoadedAsset = SoftPath.LoadSynchronous();
	    
					if (LoadedAsset)
					{
						FEnemySquadData SelectedData;
						if (LoadedAsset->GetRandomSquadForZone(PS->GetCurrentZoneTag(), SelectedData))
						{
							CombatManagerComponent->EnqueueCombatPhase(PS, &SelectedData);
						}
					}
				}
				else
				{
					CombatManagerComponent->EnqueueCombatPhase(PS);
				}
			}
		}
		
		CombatManagerComponent->MatchingCombatUser(true);
		CombatManagerComponent->StartCountingCombat();
	}
}

void ANGInGameMode::OnGameStart()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	NotifyGameStartToPlayer(GS);

	if (GS)
	{
		GS->CurrentTurn = 0;
		StartTownSelection();
	}
}

void ANGInGameMode::NotifyGameStartToPlayer(ANGGameState* GS)
{
	if (!GS)	return;
	
	for (APlayerState* RawPS : GS->PlayerArray)
	{
		if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
		{
			PS->SetGameState(EGameState::Exploration);
		}
	}	
}

void ANGInGameMode::OnCombatFinished(const FCombatResultData& ResultData)
{
	UE_LOG(LogTemp, Log, TEXT("Combat Finished"));
		
	//ResultData로 점수나 그런거 반영하기
	
	// 각 플레이어가 전투 끝났음을 표시
	CheckAllPlayersFinishedAction();
}

void ANGInGameMode::ReportPawnDeath(ANGPawnBase* DeadPawn) const
{
	if (CombatManagerComponent)
	{
		CombatManagerComponent->NotifyPawnDied(DeadPawn);
	}
}

void ANGInGameMode::StartTurn()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	GS->CurrentTurn++;
	if (GS->CurrentTurn > 10)
	{
		UE_LOG(LogTemp, Warning, TEXT("Max turns (10) reached. Ending game."));
		// TODO: Game Over 로직
		return;
	}

	for (APlayerState* RawPS : GS->PlayerArray)
	{
		if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
		{
			PS->SetHasSelectedNode(false);
			PS->SetActionFinished(false);
		}
	}

	StartNodeSelection();
}

void ANGInGameMode::StartTownSelection()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;
	
	UE_LOG(LogTemp, Warning, TEXT("=== Turn %d: Town Selection Phase Started ==="), GS->CurrentTurn);
	GS->SetGameFlow(EGameplayPhase::TownSelection, EGameTime::NodeSelectionTime);
	
	for (APlayerState* RawPS : GS->PlayerArray)
	{
		if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
		{
			PS->SetHasSelectedNode(false);
			PS->SetCurrentNodeID(-1);
			PS->SetTargetNodeID(-1);
		}
	}
	
	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::OnTownSelectionTimerTick, EGameTime::NodeSelectionTime, false);
}

void ANGInGameMode::StartNodeSelection()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	GS->SetGameFlow(EGameplayPhase::NodeSelection, EGameTime::NodeSelectionTime);
	ActiveMovementPlayerIndex = 0;
	BeginCurrentPlayerMovement();
	UE_LOG(LogTemp, Warning, TEXT("=== Turn %d: Node Selection Phase Started (%.1fs) ==="), GS->CurrentTurn, EGameTime::NodeSelectionTime);
}

void ANGInGameMode::OnTownSelectionTimerTick()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	const FMapNodeData* FallbackTownNode = GS->MapNodes.FindByPredicate([](const FMapNodeData& Node)
	{
		return Node.NodeType == ENodeType::Town;
	});
	
	if (!FallbackTownNode)
	{
		UE_LOG(LogTemp, Error, TEXT("No fallback town node found."));
		return;
	}
	
	for (APlayerState* RawPS : GS->PlayerArray)
	{
		ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS);
		if (!PS || PS->HasSelectedNode())
		{
			continue;
		}

		PS->SetCurrentNodeID(FallbackTownNode->NodeID);
		PS->SetTargetNodeID(FallbackTownNode->NodeID);
		PS->SetHasSelectedNode(true);
		
		// Todo: 마을 버프 적용
		// ApplyTownBuff(PS, *FallbackTownNode);
		
		UE_LOG(LogTemp, Warning, TEXT("Town Selection: Player %s selected fallback town node (%d)"), *PS->GetPlayerName(), FallbackTownNode->NodeID);
	}
	
	StartTurn();
}

void ANGInGameMode::OnNodeSelectionTimerTick()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS || GS->CurrentPhase != EGameplayPhase::NodeSelection) return;

	CompleteCurrentPlayerMovement(true);
}

void ANGInGameMode::RollMovementDice(AController* Controller)
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	ANGPlayerState* PS = Controller ? Controller->GetPlayerState<ANGPlayerState>() : nullptr;
	if (!GS || !PS
		|| GS->CurrentPhase != EGameplayPhase::NodeSelection
		|| GS->ActiveMovementPlayer != PS
		|| GS->DiceResult != 0
		|| PS->HasSelectedNode())
	{
		return;
	}

	const int32 DiceResult = FMath::RandRange(1, 6);
	const TArray<int32> ReachableNodeIDs = FindReachableNodeIDs(PS->GetCurrentNodeID(), DiceResult);
	GS->SetMovementTurn(PS, DiceResult, ReachableNodeIDs);

	UE_LOG(LogTemp, Log, TEXT("Player %s rolled %d (%d reachable nodes)."),
		*PS->GetPlayerName(), DiceResult, ReachableNodeIDs.Num());
}

void ANGInGameMode::ProcessNodeSelection(AController* Controller, int32 NodeID)
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	ANGPlayerState* PS = Controller ? Controller->GetPlayerState<ANGPlayerState>() : nullptr;
	if (!GS || !PS || PS->HasSelectedNode() || NodeID < 0) return;
	
	const FMapNodeData* NodeData = GS->MapNodes.FindByPredicate([NodeID](const FMapNodeData& Data) {
		return Data.NodeID == NodeID;
	});

	if (!NodeData) return;
	
	// 분기 1: 시작 마을 선택
	if (GS->CurrentPhase == EGameplayPhase::TownSelection)
	{
		if (NodeData->NodeType != ENodeType::Town) return;
		PS->SetCurrentNodeID(NodeID);
		PS->SetTargetNodeID(NodeID);
		PS->SetHasSelectedNode(true);
		
		// Todo: 시작 마을 버프 적용
		// ApplyTownBuff(PS, *Node);
		CheckAllPlayersReadyForNodeSelection();
		return;
	}
	
	// 분기 2: 일반 노드 선택
	if (GS->CurrentPhase == EGameplayPhase::NodeSelection)
	{
		if (GS->ActiveMovementPlayer != PS
			|| GS->DiceResult <= 0
			|| !GS->ReachableNodeIDs.Contains(NodeID))
		{
			return;
		}
		
		PS->SetTargetNodeID(NodeID);
		PS->SetHasSelectedNode(true);
		AdvanceMovementPlayer();
	}
}

void ANGInGameMode::BeginCurrentPlayerMovement()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS || GS->CurrentPhase != EGameplayPhase::NodeSelection) return;

	while (GS->PlayerArray.IsValidIndex(ActiveMovementPlayerIndex))
	{
		ANGPlayerState* PS = Cast<ANGPlayerState>(GS->PlayerArray[ActiveMovementPlayerIndex]);
		if (PS)
		{
			GS->SetMovementTurn(PS, 0, TArray<int32>());
			GS->SetGameFlow(EGameplayPhase::NodeSelection, EGameTime::NodeSelectionTime);
			GetWorldTimerManager().SetTimer(PhaseTimerHandle, this,
				&ThisClass::OnNodeSelectionTimerTick, EGameTime::NodeSelectionTime, false);
			return;
		}

		++ActiveMovementPlayerIndex;
	}

	GS->SetMovementTurn(nullptr, 0, TArray<int32>());
	StartActionPhase();
}

void ANGInGameMode::AdvanceMovementPlayer()
{
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	++ActiveMovementPlayerIndex;
	BeginCurrentPlayerMovement();
}

void ANGInGameMode::CompleteCurrentPlayerMovement(bool bStayOnCurrentNode)
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	ANGPlayerState* PS = GS ? GS->ActiveMovementPlayer.Get() : nullptr;
	if (!GS || !PS) return;

	if (bStayOnCurrentNode)
	{
		PS->SetTargetNodeID(PS->GetCurrentNodeID());
	}

	PS->SetHasSelectedNode(true);
	AdvanceMovementPlayer();
}

TArray<int32> ANGInGameMode::FindReachableNodeIDs(int32 StartNodeID, int32 MaxDistance) const
{
	TArray<int32> Result;
	const ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS || StartNodeID < 0 || MaxDistance <= 0) return Result;

	TMap<int32, int32> Distances;
	TArray<int32> Queue;
	Distances.Add(StartNodeID, 0);
	Queue.Add(StartNodeID);

	for (int32 QueueIndex = 0; QueueIndex < Queue.Num(); ++QueueIndex)
	{
		const int32 CurrentNodeID = Queue[QueueIndex];
		const int32 CurrentDistance = Distances[CurrentNodeID];
		if (CurrentDistance >= MaxDistance) continue;

		const FMapNodeData* CurrentNode = GS->MapNodes.FindByPredicate(
			[CurrentNodeID](const FMapNodeData& Node)
			{
				return Node.NodeID == CurrentNodeID;
			});
		if (!CurrentNode) continue;

		for (int32 ConnectedNodeID : CurrentNode->ConnectedNodeIDs)
		{
			if (Distances.Contains(ConnectedNodeID)) continue;

			const int32 ConnectedDistance = CurrentDistance + 1;
			Distances.Add(ConnectedNodeID, ConnectedDistance);
			Queue.Add(ConnectedNodeID);
			Result.Add(ConnectedNodeID);
		}
	}

	return Result;
}

void ANGInGameMode::CheckAllPlayersReadyForNodeSelection()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	for (APlayerState* RawPS : GS->PlayerArray)
	{
		if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
		{
			if (!PS->HasSelectedNode())
			{
				return;
			}
		}
	}
	
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	
	if (GS->CurrentPhase == EGameplayPhase::TownSelection)
	{
		StartTurn();
	}
	else
	{
		StartActionPhase();
	}
}

void ANGInGameMode::StartActionPhase()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	GS->SetMovementTurn(nullptr, 0, TArray<int32>());
	GS->SetGameFlow(EGameplayPhase::ActionPhase, EGameTime::ActionPhaseTime);

	UE_LOG(LogTemp, Warning, TEXT("=== Action Phase Started (%f s) ==="), EGameTime::ActionPhaseTime);

	TArray<ANGPlayerState*> PlayingPlayers;
	for (APlayerState* RawPS : GS->PlayerArray)
	{
		if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
		{
			PlayingPlayers.Add(PS);
		}
	}

	TArray<ANGPlayerState*> HandledPlayers;

	// 거리 계산 로직 (거리 <= 1칸)
	// 현재 맵 제네레이터 정보가 없으므로 ID 차이가 1 이하인 경우로 임시 판별 (이후 ConnectedNodes 로직으로 대체 필요)
	for (int32 i = 0; i < PlayingPlayers.Num(); ++i)
	{
		ANGPlayerState* PlayerA = PlayingPlayers[i];
		if (HandledPlayers.Contains(PlayerA)) continue;

		for (int32 j = i + 1; j < PlayingPlayers.Num(); ++j)
		{
			ANGPlayerState* PlayerB = PlayingPlayers[j];
			if (HandledPlayers.Contains(PlayerB)) continue;

			// 임시 인접 판별 로직
			bool bIsAdjacent = FMath::Abs(PlayerA->GetTargetNodeID() - PlayerB->GetTargetNodeID()) <= 1;

			if (bIsAdjacent)
			{
				UE_LOG(LogTemp, Warning, TEXT("PvP Matched: Player %s vs Player %s"), *PlayerA->GetPlayerName(), *PlayerB->GetPlayerName());
				CombatManagerComponent->EnqueueCombatPhase(PlayerA);
				CombatManagerComponent->EnqueueCombatPhase(PlayerB);
				HandledPlayers.Add(PlayerA);
				HandledPlayers.Add(PlayerB);
				break;
			}
		}
	}

	// 1:1:1 등 홀수 상황에서 남은 유저는 CPU 전투(복제 유닛 전투) 진행
	for (ANGPlayerState* PS : PlayingPlayers)
	{
		if (!HandledPlayers.Contains(PS))
		{
			UE_LOG(LogTemp, Warning, TEXT("CPU Match (Clone): Player %s"), *PS->GetPlayerName());
			// 원래는 1vs1 대상의 복제 유닛 데이터를 Enqueue 해야 함. 임시로 CPU전 세팅
			TSoftObjectPtr<UNGEnemyDataAsset> SoftPath = GetDefault<UNGDeveloperSettings>()->EnemyDataAsset;
			UNGEnemyDataAsset* LoadedAsset = SoftPath.LoadSynchronous();
			if (LoadedAsset)
			{
				FEnemySquadData SelectedData;
				if (LoadedAsset->GetRandomSquadForZone(PS->GetCurrentZoneTag(), SelectedData))
				{
					CombatManagerComponent->EnqueueCombatPhase(PS, &SelectedData);
				}
			}
			HandledPlayers.Add(PS);
		}
	}

	// 노드 이동 적용
	for (ANGPlayerState* PS : PlayingPlayers)
	{
		PS->SetCurrentNodeID(PS->GetTargetNodeID());
	}

	CombatManagerComponent->MatchingCombatUser(false);
	CombatManagerComponent->StartCountingCombat();

	GetWorldTimerManager().SetTimer(PhaseTimerHandle, this, &ThisClass::OnActionPhaseTimerTick, EGameTime::ActionPhaseTime, false);
}

void ANGInGameMode::OnActionPhaseTimerTick()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	EndTurn();
}

void ANGInGameMode::CheckAllPlayersFinishedAction()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	for (APlayerState* RawPS : GS->PlayerArray)
	{
		if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
		{
			if (!PS->IsActionFinished())
			{
				return;
			}
		}
	}
	
	GetWorldTimerManager().ClearTimer(PhaseTimerHandle);
	EndTurn();
}

void ANGInGameMode::EndTurn()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	if (!GS) return;

	GS->SetGameFlow(EGameplayPhase::TurnEnd, 0.f);
	UE_LOG(LogTemp, Warning, TEXT("=== Turn %d Ended ==="), GS->CurrentTurn);
	
	StartTurn();
}

void ANGInGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Only server
	InitializeUnitPool();
	
	// Create Map
	if (MapGeneratorComponent)
	{
		// 임의의 시드값으로 맵 생성
		int32 Seed = FMath::Rand();
		MapGeneratorComponent->GenerateMap(Seed);

		// 생성된 맵 데이터를 GameState에 전달 (이를 통해 모든 클라이언트에게 동기화됨)
		if (ANGGameState* NGGameState = GetGameState<ANGGameState>())
		{
			NGGameState->SetMapNodes(MapGeneratorComponent->GetGeneratedNodes());
		}
	}
	
	// Todo: 현재 게임 시작 시 바로 호출되어 모든 플레이어의 화면이 정상적이지 않은 상태에서 시작함.
	// 따라서, 앞 선 로딩화면이나, 시작 연출을 표시하고 넘어가는 로직이 필요.
	// 생성되었을 경우 밑의 OnGameStart 함수는 다른 곳에서 호출할 수 있도록.
	OnGameStart();
}

int32 ANGInGameMode::SellUnit(ANGUnitPawn* Unit)
{
	int32 UnitSellValue = 1;
	
	//DataTable참조해서 유닛의 가격이랑 성보고 돈 계산 후 반환
	
	return UnitSellValue;
}

int32 ANGInGameMode::GrabUnitFromPool(FGameplayTag UnitTag)
{
	if (!HasAuthority()) return -1;
	
	if (int32* Count = UnitPool.Find(UnitTag))
	{
		if (*Count > 0)
		{
			(*Count)--;
			return *Count;
		}
	}
	return -1;
}

bool ANGInGameMode::IsExistUnit(FGameplayTag UnitTag)
{
	if (int32* Count = UnitPool.Find(UnitTag))
	{
		return *Count > 0;
	}
	return false;
}

bool ANGInGameMode::IsExistUnitDataTable() const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UNGPawnDataManager* UnitDataManager = GI->GetSubsystem<UNGPawnDataManager>())
		{
			return UnitDataManager->IsExistUnitDataTable();
		}
	}
	return false;
}

void ANGInGameMode::ReturnUnitToPool(FGameplayTag UnitTag, int32 UnitCount)
{
	if (int32* Count = UnitPool.Find(UnitTag))
	{
		(*Count) += UnitCount;
	}
}

FGameplayTag ANGInGameMode::GetRandomUnitByTier(EUnitTier Tier)
{
	if (TieredUnitPool.Contains(Tier))
	{
		TArray<FGameplayTag> AvailableUnits;
		const TArray<FGameplayTag>& UnitsInTier = TieredUnitPool[Tier];

		// 현재 남아 있는 유닛만 필터링
		for (FGameplayTag UnitTag : UnitsInTier)
		{
			if (int32* Count = UnitPool.Find(UnitTag))
			{
				if (*Count > 0)
				{
					AvailableUnits.Add(UnitTag);
				}
			}
		}

		if (AvailableUnits.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, AvailableUnits.Num() - 1);
			return AvailableUnits[RandomIndex];
		}
	}
	return FGameplayTag();
}

TSubclassOf<ANGUnitPawn> ANGInGameMode::GetUnitClass(FGameplayTag UnitTag) const
{
	const FUnitData* FoundRow = GetUnitData(UnitTag);
	if (!FoundRow)	return nullptr;
	
	return FoundRow->UnitClass;
}

float ANGInGameMode::GetUnitPrice(ANGUnitPawn* Unit) const
{
	const FUnitData* UnitData = GetUnitData(Unit->GetIdentificationTag());
	if (!UnitData) return 0.f;
	
	float UnitSellValue = UnitData->Price;
	UE_LOG(LogTemp, Log, TEXT("Price: %f"), UnitSellValue);
	
	return UnitSellValue;
}

bool ANGInGameMode::CanBuyUnit(FGameplayTag UnitTag, float OwnedGold) const
{
	const FUnitData* UnitData = GetUnitData(UnitTag);
	if (!UnitData) return false;
	
	if (UnitData->Price > OwnedGold)	return false;
	
	return true;
}

const FUnitData* ANGInGameMode::GetUnitData(FGameplayTag UnitTag) const
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UNGPawnDataManager* UnitDataManager = GI->GetSubsystem<UNGPawnDataManager>())
		{
			return UnitDataManager->GetUnitData(UnitTag);
		}
	}
	return nullptr;
}

void ANGInGameMode::InitializeUnitPool()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UNGPawnDataManager* UnitDataManager = GI->GetSubsystem<UNGPawnDataManager>())
		{
			const TMap<FGameplayTag, FUnitData*>& AllUnitData = UnitDataManager->GetAllUnitDataMap();
			for (const auto& Pair : AllUnitData)
			{
				FGameplayTag UnitTag = Pair.Key;
				FUnitData* Row = Pair.Value;

				if (Row && Row->UnitClass)
				{
					// 총 개수 추가
					UnitPool.Add(UnitTag, Row->TotalCountInPool);

					// 등급별로 유닛 클래스 추가
					TieredUnitPool.FindOrAdd(Row->Tier).Add(UnitTag);
				}
			}
			UE_LOG(LogTemp, Warning, TEXT("Unit Pool Initialized on Server using UnitDataManager."));
		}
	}
}
