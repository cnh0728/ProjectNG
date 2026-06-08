// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Pawn/NGPawnBase.h"
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
	void FindPath(const FGridAddress& StartAddress, const FGridAddress& TargetAddress, TArray<FIntVector2>& OutPath);

	UFUNCTION(Category = "Grid|PathFinding")
	ANGPawnBase* FindPathToClosestEnemy(const FGridAddress& StartAddress, uint32 OwnerIndex, TArray<FIntVector2>& OutPath);
	
protected:

	// A* 비용 계산용 휴리스틱 (UGridMapHelper의 GetDistance 활용)
	int32 GetHeuristicCost(const FIntVector2& A, const FIntVector2& B) const;
};
