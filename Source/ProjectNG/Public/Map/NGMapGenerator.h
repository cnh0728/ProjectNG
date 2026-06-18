// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NGMapTypes.h"
#include "NGMapGenerator.generated.h"


class UMapNodeDataAsset;
class ANGMapNode;

struct FNodeDistanceInfo
{
	int32 TargetIndex;
	float DistSquared;
	
	bool operator<(const FNodeDistanceInfo& Other) const
	{
		return DistSquared < Other.DistSquared;
	}
};

UCLASS()
class PROJECTNG_API ANGMapGenerator : public AActor
{
	GENERATED_BODY()
	
public:
	ANGMapGenerator();

	void GenerateMap(int32 Seed);
	
	FGameplayTag GetGameplayTagById(int32 NodeID);
	
	const TArray<FMapNodeData>& GetGeneratedNodes() const { return GeneratedNodes; };

protected:
	virtual void BeginPlay() override;

private:
	void InitializedNodes();
	
	void SpawnNodes();

	void PreInitializeValidPoints();
	

	void ConnectionNodes();
	
	void DrawPathVisual();
	
public:
	UPROPERTY(EditAnywhere, Category = "Map|Data")
	TObjectPtr<UMapNodeDataAsset> MapNodeData;
	
	UPROPERTY(EditAnywhere, Category = "Map|Data")
	TObjectPtr<UTexture2D> MapMaskTexture;
	
	UPROPERTY(EditDefaultsOnly, Category = "Map|Visual")
	TObjectPtr<UStaticMesh> PathMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Map|Visual")
	TObjectPtr<UMaterialInterface> PathMaterial;
	
	// 맵의 전체 크기 (반지름)
	UPROPERTY(EditAnywhere, Category = "Map|Config")
	float MapRadius = 1000.f;

	// 마을 개수
	UPROPERTY(EditAnywhere, Category = "Map|Config")
	int32 NumberOfTowns = 4;
	
	// 최대 노드 개수
	UPROPERTY(EditAnywhere, Category = "Map|Config", meta = (ToolTip = "마을 제외 최대 노드 생성 개수"))
	int32 MaxNodeCount = 12;
	
	UPROPERTY(EditAnywhere, Category = "Map|Config")
	float MinNodeDistance = 50.0f;
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Map")
	int32 RandomSeed;
	
private:
	TArray<FMapNodeData> GeneratedNodes;
	
	TArray<FVector2D> ValidSpawnPoints;
	
	FRandomStream RandomStream;
};
