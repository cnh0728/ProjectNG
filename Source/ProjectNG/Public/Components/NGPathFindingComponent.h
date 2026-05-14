// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NGPathFindingComponent.generated.h"


struct FGridAddress;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTNG_API UNGPathFindingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNGPathFindingComponent();

	/**
	 * StartAddress에서 TargetAddress까지의 최단 경로를 찾아 반환합니다.
	 * @return 목적지까지의 타일 인덱스 배열 (시작점은 제외, 목적지는 포함)
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid|PathFinding")
	TArray<FIntVector2> FindPath(const FGridAddress& StartAddress, const FGridAddress& TargetAddress);

	UFUNCTION(Category = "Grid|PathFinding")
	TArray<FIntVector2> FindPathToClosestEnemy(const FGridAddress& StartAddress, uint32 OwnerIndex);
	
	// 육각형(Odd-Row 기반) 이웃 타일 6방향 반환
	TArray<FIntVector2> GetHexNeighbors(const FIntVector2& CurrentIndex) const;
	
protected:

	// A* 비용 계산용 휴리스틱 (UGridMapHelper의 GetDistance 활용)
	int32 GetHeuristicCost(const FIntVector2& A, const FIntVector2& B) const;
};
