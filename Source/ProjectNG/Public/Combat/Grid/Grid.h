// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Grid.generated.h"

/**
 * 
 */

class ANGPlayerState;
class ANGUnitPawn;

UENUM(BlueprintType)
enum class EGridType : uint8 {
	None,
	Combat,
	Wait,
	EnemyWait,
};

USTRUCT(BlueprintType)
struct FGridAddress
{
	GENERATED_BODY()
	
	UPROPERTY()
	FIntVector2 GridIndex;

	UPROPERTY()
	EGridType GridType;
	
	UPROPERTY()
	TObjectPtr<ANGPlayerState> GridOwnerPS;
};

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
struct PROJECTNG_API FGridMapBase
{
	GENERATED_BODY()

	void Internal_Initialize(int32 InW, int32 InH, float InCellSize, const FVector& InPivot);
	
	bool IsValidIndex(const FIntVector2 GridIndex) const;
	int32 ConvertPointToIndex(const FIntVector2 GridIndex) const;
	FIntVector2 ConvertIndexToPoint(int32 Index) const;

	void SetGridData(FIntVector2 GridIndex, const FGridData& GridData);
	void EmptyGridMap(const FIntVector2& GridIndex);
	FGridData GetGridData(const FIntVector2 GridIndex) const;
    
	void ResetGridInfo();
	void ResetEmptyGridIndex();
    
	bool IsGridIndexEmpty(const FIntVector2& GridIndex) const;
	bool IsPossibleSpawnPawn() const;
	TOptional<FIntVector2> GetEmptyGridIndex() const;
	const TArray<FGridData>& GetGridInfo() const { return GridInfo; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CellSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Pivot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGridType GridType;

	UPROPERTY()
	TArray<FIntVector2> EmptyGridIndex;

	UPROPERTY()
	TArray<FGridData> GridInfo;

	UPROPERTY()
	float Offset;
};

USTRUCT(BlueprintType)
struct PROJECTNG_API FHexGridMap : public FGridMapBase
{
	GENERATED_BODY()

	FHexGridMap();
	
	void InitializeMap(int32 InW, int32 InH, float InCellSize, const FVector& InPivot);
	
};

USTRUCT(BlueprintType)
struct PROJECTNG_API FQuadGridMap : public FGridMapBase
{
	GENERATED_BODY()

	FQuadGridMap();
	void InitializeMap(int32 InW, int32 InH, float InCellSize, const FVector& InPivot);
	
};

UCLASS()
class PROJECTNG_API UGridMapHelper : public UObject
{
	GENERATED_BODY()

public:
	static FVector GetRelativeLocation(FGridAddress GridAddress);
	static FVector GetWorldLocation(FGridAddress GridAddress);
	static FIntVector2 GetCellIndex(EGridType GridType, const FVector& Location, ANGPlayerState* PS);
	
	static FGridMapBase* GetGridMap(FGridAddress GridAddress);

	static FVector GetHexCorner(const FHexGridMap& GridMap, int32 Index);
	static const FIntVector2 GetMirroredIndex(const FGridMapBase& GridMap, FIntVector2 OriginIndex);
    
	static int32 GetDistance(FIntVector2 A, FIntVector2 B);
	static FIntVector2 RectToAxial(int32 Col, int32 Row);
	static FIntVector GetCubeIndex(const FIntVector2 AxialIndex);
	
	static void DrawDebugGrid(const UObject* WorldContextObject, FGridAddress GridAddress);
};