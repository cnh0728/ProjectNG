// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NGMapTypes.h"
#include "NGMapGenerator.generated.h"

class UMapNodeDataAsset;
class ANGMapNode;

UCLASS()
class PROJECTNG_API ANGMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:
	ANGMapGenerator();

	// ─── 공개 API ───
	void GenerateMap(int32 Seed);
	
	FGameplayTag GetGameplayTagById(int32 NodeID);
	
	const TArray<FMapNodeData>& GetGeneratedNodes() const { return GeneratedNodes; };

	// ─── 유틸리티 ───
	// 특정 레이어의 모든 노드 가져오기
	TArray<const FMapNodeData*> GetNodesAtLayer(int32 LayerIndex) const;
	// 시작 마을 노드들만 가져오기
	TArray<const FMapNodeData*> GetTownNodes() const;

protected:
	virtual void BeginPlay() override;

private:
	// ─── 생성 파이프라인 ───
	void GenerateLayerNodes();        // Step 1: 레이어별 노드 생성
	void ConnectLayers();             // Step 2: 레이어 간 전방 연결
	void AssignNodeTypes();           // Step 3: 노드 타입 가중치 배정
	void AssignTownBuffs();           // Step 4: 마을 버프 랜덤 배정
	void AssignNodePositions();       // Step 5: UI용 논리 좌표 배정
	bool ValidateConnectivity();      // Step 6: 도달성 검증 (BFS)

	// ─── 헬퍼 ───
	ENodeType PickRandomNodeType(int32 LayerIndex);
	int32 CalculateNodesForLayer(int32 LayerIndex);
	FGameplayTag GetTagForNodeType(ENodeType Type);

public:
	// ─── 데이터 에셋 ───
	UPROPERTY(EditAnywhere, Category = "Map|Data")
	TObjectPtr<UMapNodeDataAsset> MapNodeData;

	// ─── 레이어 설정 ───
	UPROPERTY(EditAnywhere, Category = "Map|Config")
	int32 NumberOfLayers = 11;       // 0 ~ 10 (총 11개 레이어)

	UPROPERTY(EditAnywhere, Category = "Map|Config")
	int32 NumberOfTowns = 3;         // 시작 마을 수 (고정)

	UPROPERTY(EditAnywhere, Category = "Map|Config")
	int32 MaxNodesPerLayer = 4;

	UPROPERTY(EditAnywhere, Category = "Map|Config")
	int32 MinNodesPerLayer = 2;

	// ─── 연결 설정 ───
	UPROPERTY(EditAnywhere, Category = "Map|Config")
	int32 MinConnectionsPerNode = 1;

	UPROPERTY(EditAnywhere, Category = "Map|Config")
	int32 MaxConnectionsPerNode = 3;

	// ─── 병목 레이어 (PvP 조우 보장) ───
	UPROPERTY(EditAnywhere, Category = "Map|Bottleneck", meta = (ToolTip = "PvP를 보장할 병목 레이어 인덱스. 해당 레이어는 노드 2개로 고정"))
	TArray<int32> BottleneckLayers = { 4, 7 };

	UPROPERTY(EditAnywhere, Category = "Map|Bottleneck")
	int32 BottleneckNodeCount = 2;   // 병목 레이어의 노드 수

	// ─── 노드 타입 가중치 ───
	UPROPERTY(EditAnywhere, Category = "Map|NodeWeights")
	float EmptyWeight = 0.30f;

	UPROPERTY(EditAnywhere, Category = "Map|NodeWeights")
	float ShopWeight = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Map|NodeWeights")
	float CombatWeight = 0.20f;

	UPROPERTY(EditAnywhere, Category = "Map|NodeWeights")
	float EventWeight = 0.20f;

	UPROPERTY(EditAnywhere, Category = "Map|NodeWeights")
	float RestWeight = 0.10f;

	UPROPERTY(EditAnywhere, Category = "Map|NodeWeights")
	float EliteWeight = 0.05f;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Map")
	int32 RandomSeed;

private:
	TArray<FMapNodeData> GeneratedNodes;
	FRandomStream RandomStream;
};
