// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/Grid/Grid.h"

FGridMap::FGridMap(): CountX(10), CountY(10), CellSize(100.f), Pivot(FVector::ZeroVector)
{
}

void FGridMap::InitializeMap(const int32 InSizeX, const int32 InSizeY, const float InCellSize, const FVector& InPivot)
{
	CountX = InSizeX;
	CountY = InSizeY;
	CellSize = InCellSize;
	Pivot = InPivot;

}

FVector FGridMap::GetRelativeLocation(const FIntVector2 GridIndex) const
{
	// 셀 중앙에 위치하도록 오프셋을 CellSize/2를 줌
	return FVector(GridIndex.X * CellSize + OFFSET, GridIndex.Y * CellSize + OFFSET, 0.0f);
}

FVector FGridMap::GetWorldLocation(const FIntVector2 GridIndex) const
{
	return GetRelativeLocation(GridIndex) + Pivot;
}

FIntVector2 FGridMap::GetCellIndex(const FVector& Location) const
{
	int32 GridX = FMath::FloorToInt((Location.X - Pivot.X) / CellSize);
	int32 GridY = FMath::FloorToInt((Location.Y - Pivot.Y) / CellSize);
   
	return FIntVector2(GridX, GridY);
}

FIntVector2 FGridMap::GetCellIndex(const FVector2D& Location) const
{
	int32 GridX = FMath::FloorToInt((Location.X - Pivot.X) / CellSize);
	int32 GridY = FMath::FloorToInt((Location.Y - Pivot.Y) / CellSize);
   
	return FIntVector2(GridX, GridY);
}

bool FGridMap::IsValidIndex(const FIntVector2 GridIndex) const
{
	return (GridIndex.X >= 0 && GridIndex.X < CountX) && (GridIndex.Y >= 0 && GridIndex.Y < CountY);
}

void FGridMap::ResetGridInfo()
{
	GridInfo.Reset();
}

void FGridMap::ResetEmptyGridIndex()
{	
	EmptyGridIndex.Reset();
	
	for (int32 X=0;X<CountX;++X)
	{
		for (int32 Y=0;Y<CountY;++Y)
		{
			FIntVector2 Index = FIntVector2(X,Y);
			EmptyGridIndex.AddUnique(Index);
		}
	}
}

void FGridMap::RemoveEmptyGridIndex(const FIntVector2& GridIndex)
{
	EmptyGridIndex.Remove(GridIndex);
}

void FGridMap::SetGridData(FIntVector2 GridIndex, const FGridData& GridData)
{
	GridInfo.Add(GridIndex, GridData);

	RemoveEmptyGridIndex(GridIndex);
}

void FGridMap::EmptyGridMap(const FIntVector2& GridIndex)
{
	FGridData* FoundData = GridInfo.Find(GridIndex);

	if (FoundData)
	{
		FoundData->Reset();
		EmptyGridIndex.AddUnique(GridIndex);
	}
}

FGridData FGridMap::GetGridData(const FIntVector2 GridIndex)
{
	FGridData* FoundData = GridInfo.Find(GridIndex);
    
	if (FoundData)
	{
		FGridData Result = *FoundData;
        
		// 아래 두개는 Pop아닌가?
		// FoundData->Reset();
		// EmptyGridIndex.AddUnique(GridIndex);

		return Result;
	}

	UE_LOG(LogTemp, Warning, TEXT("GridIndex %s 에 데이터가 없습니다!"), *GridIndex.ToString());
	return FGridData();
}

TOptional<FIntVector2> FGridMap::GetEmptyGridIndex()
{
	if (EmptyGridIndex.IsEmpty())
	{
		return TOptional<FIntVector2>();
	}
	
	FIntVector2 Ret = EmptyGridIndex.Last();
	
	return Ret;
}

bool FGridMap::IsGridIndexEmpty(const FIntVector2& GridIndex) const
{
	return EmptyGridIndex.Find(GridIndex) != INDEX_NONE;
}

