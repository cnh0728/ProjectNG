// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/Grid/Grid.h"

#include "Player/NGPlayerState.h"

void FGridMapBase::Internal_Initialize(int32 InW, int32 InH, float InCellSize, const FVector& InPivot)
{
    Width = InW;
    Height = InH;
    CellSize = InCellSize;
    Pivot = InPivot;
    ResetEmptyGridIndex();
}

bool FGridMapBase::IsValidIndex(const FIntVector2 GridIndex) const
{
    return (GridIndex.X >= 0 && GridIndex.X < Width) && (GridIndex.Y >= 0 && GridIndex.Y < Height);
}

int32 FGridMapBase::ConvertPointToIndex(const FIntVector2 GridIndex) const
{
    return GridIndex.Y * Width + GridIndex.X;
}

FIntVector2 FGridMapBase::ConvertIndexToPoint(const int32 Index) const
{
    return FIntVector2(Index % Width, Index / Width);
}

void FGridMapBase::SetGridData(FIntVector2 GridIndex, const FGridData& GridData)
{
    if (!IsValidIndex(GridIndex)) return;
    GridInfo[ConvertPointToIndex(GridIndex)] = GridData;
    EmptyGridIndex.Remove(GridIndex);
}

void FGridMapBase::EmptyGridMap(const FIntVector2& GridIndex)
{	
    if (!IsValidIndex(GridIndex)) return;
    int32 Idx = ConvertPointToIndex(GridIndex);
    if (GridInfo[Idx].PlacedPawn.IsValid())
    {
        GridInfo[Idx].Reset();
        EmptyGridIndex.AddUnique(GridIndex);
    }
}

FGridData FGridMapBase::GetGridData(const FIntVector2 GridIndex) const
{
    if (!IsValidIndex(GridIndex)) return FGridData();
    return GridInfo[ConvertPointToIndex(GridIndex)];
}

void FGridMapBase::ResetGridInfo() { GridInfo.Reset(); }

void FGridMapBase::ResetEmptyGridIndex()
{
    EmptyGridIndex.Reset();
    GridInfo.SetNum(Width * Height);

    for (int32 y = 0; y < Height; ++y)
    {
        for (int32 x = 0; x < Width; ++x)
        {
            FGridData EmptyData(nullptr);
            FIntVector2 CurrentIdx(x, y);
            EmptyGridIndex.Add(CurrentIdx);
            GridInfo[ConvertPointToIndex(CurrentIdx)] = EmptyData;
        }
    }
}

bool FGridMapBase::IsGridIndexEmpty(const FIntVector2& GridIndex) const
{
    return EmptyGridIndex.Contains(GridIndex);
}

bool FGridMapBase::IsPossibleSpawnPawn() const
{
    if (EmptyGridIndex.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Grid is full"));
        return false;
    }
    return true;
}

TOptional<FIntVector2> FGridMapBase::GetEmptyGridIndex() const
{
    if (EmptyGridIndex.IsEmpty()) return TOptional<FIntVector2>();
    return EmptyGridIndex.Last();
}

FQuadGridMap::FQuadGridMap() { Internal_Initialize(9, 1, 100.f, FVector::ZeroVector); Offset = 50.f; }

void FQuadGridMap::InitializeMap(const int32 InSizeX, const int32 InSizeY, const float InCellSize, const FVector& InPivot)
{
    Offset = InCellSize / 2.0f;
    Internal_Initialize(InSizeX, InSizeY, InCellSize, InPivot);
}

FHexGridMap::FHexGridMap() { Internal_Initialize(8, 8, 100.f, FVector::ZeroVector); }

void FHexGridMap::InitializeMap(int32 InSizeQ, int32 InSizeR, float InCellSize, const FVector& InPivot)
{
    Internal_Initialize(InSizeQ, InSizeR, InCellSize, InPivot);
}


FIntVector2 UGridMapHelper::AxialToRect(FIntVector2 Axial)
{
    int32 Col = Axial.X + FMath::FloorToInt(Axial.Y / 2.0f);
    int32 Row = Axial.Y;
    return FIntVector2(Col, Row);
}

FVector UGridMapHelper::GetRelativeLocation(FGridAddress GridAddress)
{
    if (FGridMapBase* GridMap = GetGridMap(GridAddress))
    {
        switch (GridAddress.GridType)
        {
        case EGridType::Combat:
            {
                int32 q = GridAddress.GridIndex.X;
                int32 r = GridAddress.GridIndex.Y - FMath::FloorToInt(GridAddress.GridIndex.X / 2.0f);

                float X = GridMap->CellSize * (3.0f / 2.0f) * q;
                float Y = GridMap->CellSize * FMath::Sqrt(3.0f) * (r + q * 0.5f);
                return FVector(X, Y, 0.0f);
            }
        case EGridType::Wait:
        case EGridType::EnemyWait:
            {
                return FVector(GridAddress.GridIndex.X * GridMap->CellSize + GridMap->Offset, GridAddress.GridIndex.Y * GridMap->CellSize + GridMap->Offset, 0.0f);
            }
        }
    }
    return FVector::ZeroVector;
}

FVector UGridMapHelper::GetWorldLocation(FGridAddress GridAddress)
{
    if (FGridMapBase* GridMap = GetGridMap(GridAddress))
    {
        return GetRelativeLocation(GridAddress) + GridMap->Pivot;
    }
    
    return FVector::ZeroVector;
}

FIntVector2 UGridMapHelper::GetCellIndex(EGridType GridType, const FVector& Location, ANGPlayerState* PS)
{
    FGridAddress GridAddress(FIntVector2::ZeroValue, GridType, PS);
    if (FGridMapBase* GridMap = GetGridMap(GridAddress))
    {
        switch (GridType)
        {
        case EGridType::Combat:
            {
                FVector RelativePos = Location - GridMap->Pivot;
                float q = (2.0f / 3.0f * RelativePos.X) / GridMap->CellSize;
                float r = (-1.0f / 3.0f * RelativePos.X + FMath::Sqrt(3.0f) / 3.0f * RelativePos.Y) / GridMap->CellSize;

                float x = q; float y = r; float z = -q - r;
                int32 rx = FMath::RoundToInt(x); int32 ry = FMath::RoundToInt(y); int32 rz = FMath::RoundToInt(z);

                float x_diff = FMath::Abs(rx - x); float y_diff = FMath::Abs(ry - y); float z_diff = FMath::Abs(rz - z);
                if (x_diff > y_diff && x_diff > z_diff) rx = -ry - rz;
                else if (y_diff > z_diff) ry = -rx - rz;

                return FIntVector2(rx, ry + FMath::FloorToInt(rx / 2.0f));
            }
        case EGridType::Wait:
        case EGridType::EnemyWait:
            {
                return FIntVector2(FMath::FloorToInt((Location.X - GridMap->Pivot.X) / GridMap->CellSize), FMath::FloorToInt((Location.Y - GridMap->Pivot.Y) / GridMap->CellSize));
            }
        }
    }
    
    
    return FIntVector2::ZeroValue;
}

FGridMapBase* UGridMapHelper::GetGridMap(FGridAddress GridAddress)
{
    if (GridAddress.GridOwnerPS)
    {
        switch (GridAddress.GridType)
        {
        case EGridType::Combat:
            {
                return &GridAddress.GridOwnerPS->GetCombatGridMap();
            }
        case EGridType::Wait:
            {
                return &GridAddress.GridOwnerPS->GetWaitGridMap();
            }
        case EGridType::EnemyWait:
            {
                return &GridAddress.GridOwnerPS->GetEnemyWaitGridMap();
            }
        }
    }else
    {
        UE_LOG(LogTemp, Error, TEXT("UGridMapHelper::GetGridMap - OwnerPS is nullptr!!!"));
    }
    
    return nullptr;
}

FVector UGridMapHelper::GetHexCorner(const FHexGridMap& GridMap, int32 Index)
{
    float AngleRad = FMath::DegreesToRadians(60.0f * Index);
    return FVector(GridMap.CellSize * FMath::Cos(AngleRad), GridMap.CellSize * FMath::Sin(AngleRad), 0.0f);
}

const FIntVector2 UGridMapHelper::GetMirroredIndex(const FGridMapBase& GridMap, FIntVector2 OriginIndex)
{
    return FIntVector2(GridMap.Width - 1 - OriginIndex.X, GridMap.Height - 1 - OriginIndex.Y);
}

int32 UGridMapHelper::GetDistance(FIntVector2 A, FIntVector2 B)
{
    FIntVector ACube = GetCubeIndex(A);
    FIntVector BCube = GetCubeIndex(B);
    return (FMath::Abs(ACube.X - BCube.X) + FMath::Abs(ACube.Y - BCube.Y) + FMath::Abs(ACube.Z - BCube.Z)) / 2;
}

FIntVector2 UGridMapHelper::RectToAxial(int32 Col, int32 Row)
{
    int32 q = Col - FMath::FloorToInt(Row / 2.0f);
    int32 r = Row;
    return FIntVector2(q, r);
}

