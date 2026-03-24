// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Grid.generated.h"

/**
 * 
 */

#define OFFSET (CellSize / 2)

class ANGUnitCharacter;

USTRUCT(BlueprintType)
struct FGridData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ANGUnitCharacter> PlacedCharacter;
	
	void Reset()
	{
		PlacedCharacter = nullptr;
	}
};

USTRUCT(BlueprintType)
struct PROJECTNG_API FGridMap
{
	GENERATED_BODY()
	
public:
	FGridMap();
	void InitializeMap(int32 InSizeX, int32 InSizeY, float InCellSize, const FVector& InPivot = FVector::ZeroVector);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CountX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CountY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CellSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Pivot;
	
	// 특정 그리드 인덱스(GridX, GridY)를 로컬 벡터로 변환
	FVector GetRelativeLocation(const FIntVector2 GridIndex) const;
	FVector GetWorldLocation(const FIntVector2 GridIndex) const;
	FIntVector2 GetCellIndex(const FVector& Location) const;
	FIntVector2 GetCellIndex(const FVector2D& Location) const;
	
	// 인덱스가 유효한지 검사
	bool IsValidIndex(const FIntVector2 GridIndex) const;	
	void SetGridData(FIntVector2 GridIndex, const FGridData& GridData);
	void EmptyGridMap(const FIntVector2& GridIndex);
	void ResetGridInfo();

	void RemoveEmptyGridIndex(const FIntVector2& GridIndex);
	void ResetEmptyGridIndex();
	FGridData GetGridData(const FIntVector2 GridIndex);
	TOptional<FIntVector2> GetEmptyGridIndex();
	bool IsGridIndexEmpty(const FIntVector2& GridIndex) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FIntVector2> EmptyGridIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<FIntVector2, FGridData> GridInfo;
};