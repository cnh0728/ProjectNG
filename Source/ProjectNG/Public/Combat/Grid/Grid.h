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
	TWeakObjectPtr<ANGUnitPawn> PlacedPawn;
	
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
	FVector GetRelativeLocation(FIntVector2 GridIndex) const;
	FVector GetWorldLocation(const FIntVector2 GridIndex) const;
	FIntVector2 RectToAxial(int32 Col, int32 Row) const;
	FIntVector2 AxialToRect(FIntVector2 Axial) const;
	FIntVector2 GetCellIndex(const FVector& Location) const;
    
	FVector GetHexCorner(int32 Index) const;
	const FIntVector2 GetMirroredIndex(FIntVector2 OriginIndex) const;
	
	// Axial -> Cube 변환 (거리 계산용)
	static FIntVector GetCubeIndex(const FIntVector2 AxialIndex) { return FIntVector(AxialIndex.X, AxialIndex.Y, -AxialIndex.X - AxialIndex.Y); }
	static int32 GetDistance(FIntVector2 A, FIntVector2 B);

	bool IsValidIndex(const FIntVector2 GridIndex) const;  
	void SetGridData(FIntVector2 GridIndex, const FGridData& GridData);
	void EmptyGridMap(const FIntVector2& GridIndex);
	void ResetGridInfo();
	void ResetEmptyGridIndex();
	FGridData GetGridData(const FIntVector2 GridIndex);
	TOptional<FIntVector2> GetEmptyGridIndex() const;
	bool IsGridIndexEmpty(const FIntVector2& GridIndex) const;

	bool IsPossibleSpawnPawn() const;

	const TArray<FGridData>& GetGridInfo() const;
	
protected:
	int ConvertPointToIndex(const FIntVector2 GridIndex) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FIntVector2> EmptyGridIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FGridData> GridInfo;
};

USTRUCT(BlueprintType)
struct PROJECTNG_API FQuadGridMap
{
	GENERATED_BODY()
	
public:
	FQuadGridMap();
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
	int ConvertPointToIndex(const FIntVector2 GridIndex) const;
	void ResetEmptyGridIndex();
	FGridData GetGridData(const FIntVector2 GridIndex);
	TOptional<FIntVector2> GetEmptyGridIndex();
	bool IsGridIndexEmpty(const FIntVector2& GridIndex) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FIntVector2> EmptyGridIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FGridData> GridInfo;
	
	UPROPERTY()
	float Offset;
};