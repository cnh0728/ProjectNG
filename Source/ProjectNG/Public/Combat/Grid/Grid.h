// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Grid.generated.h"

/**
 * 
 */

class ANGUnitPawn;

USTRUCT(BlueprintType)
struct FGridData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ANGUnitPawn> PlacedPawn;
	
	void Reset()
	{
		PlacedPawn = nullptr;
	}
};

USTRUCT(BlueprintType)
struct PROJECTNG_API FHexGridMap
{
	GENERATED_BODY()
    
public:
	FHexGridMap();

	void InitializeMap(int32 InSizeQ, int32 InSizeR, float InCellSize, const FVector& InPivot = FVector::ZeroVector);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CountQ; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CountR; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CellSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Pivot;

	// 핵심 좌표 변환
	FVector GetWorldLocation(const FIntVector2 GridIndex) const;
	FIntVector2 RectToAxial(int32 Col, int32 Row) const;
	FIntVector2 AxialToRect(FIntVector2 Axial) const;
	FIntVector2 GetCellIndex(const FVector& Location) const;
    
	// Axial -> Cube 변환 (거리 계산용)
	static FIntVector GetCubeIndex(const FIntVector2 AxialIndex) { return FIntVector(AxialIndex.X, AxialIndex.Y, -AxialIndex.X - AxialIndex.Y); }
	static int32 GetDistance(FIntVector2 A, FIntVector2 B);

	bool IsValidIndex(const FIntVector2 GridIndex) const;  
	void SetGridData(FIntVector2 GridIndex, const FGridData& GridData);
	void EmptyGridMap(const FIntVector2& GridIndex);
	void ResetGridInfo();
	void ResetEmptyGridIndex();
	FGridData GetGridData(const FIntVector2 GridIndex);
	TOptional<FIntVector2> GetEmptyGridIndex();
	bool IsGridIndexEmpty(const FIntVector2& GridIndex) const;

	const TMap<FIntVector2, FGridData>& GetGridInfo() const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FIntVector2> EmptyGridIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TMap<FIntVector2, FGridData> GridInfo;
};
