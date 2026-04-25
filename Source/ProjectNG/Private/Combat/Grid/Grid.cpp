// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/Grid/Grid.h"

FHexGridMap::FHexGridMap() : CountQ(10), CountR(10), CellSize(100.f), Pivot(FVector::ZeroVector)
{
}

void FHexGridMap::InitializeMap(int32 InSizeQ, int32 InSizeR, float InCellSize, const FVector& InPivot)
{
    CountQ = InSizeQ;
    CountR = InSizeR;
    CellSize = InCellSize;
    Pivot = InPivot;
}

FVector FHexGridMap::GetWorldLocation(const FIntVector2 GridIndex) const
{
    // Rect(Col, Row) -> Axial(q, r) 역보정 (세로 지그재그용)
    int32 q = GridIndex.X;
    int32 r = GridIndex.Y - FMath::FloorToInt(GridIndex.X / 2.0f);

    // Flat-top 공식 적용
    float X = CellSize * (3.0f / 2.0f) * q;
    float Y = CellSize * FMath::Sqrt(3.0f) * (r + q * 0.5f);

    return FVector(X, Y, 0.0f) + Pivot;
}

FIntVector2 FHexGridMap::RectToAxial(int32 Col, int32 Row) const
{
    int32 q = Col - FMath::FloorToInt(Row / 2.0f);
    int32 r = Row;
    return FIntVector2(q, r);
}

FIntVector2 FHexGridMap::AxialToRect(FIntVector2 Axial) const
{
    int32 Col = Axial.X + FMath::FloorToInt(Axial.Y / 2.0f);
    int32 Row = Axial.Y;
    return FIntVector2(Col, Row);
}

FIntVector2 FHexGridMap::GetCellIndex(const FVector& Location) const
{    
    FVector RelativePos = Location - Pivot;

    // 1. Flat-top 역행렬 계산 (World -> Axial)
    float q = (2.0f / 3.0f * RelativePos.X) / CellSize;
    float r = (-1.0f / 3.0f * RelativePos.X + FMath::Sqrt(3.0f) / 3.0f * RelativePos.Y) / CellSize;

    // 2. Hex Rounding (가장 가까운 육각형 중심 찾기)
    float x = q;
    float y = r;
    float z = -q - r;

    int32 rx = FMath::RoundToInt(x);
    int32 ry = FMath::RoundToInt(y);
    int32 rz = FMath::RoundToInt(z);

    float x_diff = FMath::Abs(rx - x);
    float y_diff = FMath::Abs(ry - y);
    float z_diff = FMath::Abs(rz - z);

    if (x_diff > y_diff && x_diff > z_diff) rx = -ry - rz;
    else if (y_diff > z_diff) ry = -rx - rz;

    // 3. 핵심: Axial(rx, ry)를 직사각형 인덱스(Col, Row)로 변환
    // rx(q)는 가로 번호(Col)가 되고, ry(r)은 x 오프셋만큼 더해줘야 세로 번호(Row)가 직사각형처럼 나옵니다.
    int32 Col = rx;
    int32 Row = ry + FMath::FloorToInt(rx / 2.0f);

    return FIntVector2(Col, Row);
}

int32 FHexGridMap::GetDistance(FIntVector2 A, FIntVector2 B)
{
    FIntVector ACube = GetCubeIndex(A);
    FIntVector BCube = GetCubeIndex(B);
    return (FMath::Abs(ACube.X - BCube.X) + FMath::Abs(ACube.Y - BCube.Y) + FMath::Abs(ACube.Z - BCube.Z)) / 2;
}

bool FHexGridMap::IsValidIndex(const FIntVector2 GridIndex) const
{
    // Axial 좌표계에서는 맵의 모양에 따라 유효 범위가 달라질 수 있으나, 
    // 여기서는 간단하게 Rectangular 영역 내의 Axial 인덱스만 허용합니다.
    return (GridIndex.X >= 0 && GridIndex.X < CountQ) && (GridIndex.Y >= 0 && GridIndex.Y < CountR);
}

void FHexGridMap::ResetGridInfo() { GridInfo.Reset(); }

void FHexGridMap::ResetEmptyGridIndex()
{
    EmptyGridIndex.Reset();
    for (int32 q = 0; q < CountQ; ++q)
    {
        for (int32 r = 0; r < CountR; ++r)
        {
            EmptyGridIndex.Add(FIntVector2(q, r));
        }
    }
    
    for (auto a : EmptyGridIndex)
    {
        UE_LOG(LogTemp, Log, TEXT("EmptyIndex: %d %d"), a.X, a.Y);
    }
}

void FHexGridMap::SetGridData(FIntVector2 GridIndex, const FGridData& GridData)
{
    GridInfo.Add(GridIndex, GridData);
    EmptyGridIndex.Remove(GridIndex);
}

void FHexGridMap::EmptyGridMap(const FIntVector2& GridIndex)
{
    if (FGridData* FoundData = GridInfo.Find(GridIndex))
    {
        FoundData->Reset();
        EmptyGridIndex.AddUnique(GridIndex);
    }
}

FGridData FHexGridMap::GetGridData(const FIntVector2 GridIndex)
{
    if (FGridData* FoundData = GridInfo.Find(GridIndex)) return *FoundData;
    return FGridData();
}

TOptional<FIntVector2> FHexGridMap::GetEmptyGridIndex()
{
    if (EmptyGridIndex.IsEmpty()) return TOptional<FIntVector2>();
    return EmptyGridIndex.Last();
}

bool FHexGridMap::IsGridIndexEmpty(const FIntVector2& GridIndex) const
{
    return EmptyGridIndex.Contains(GridIndex);
}

const TMap<FIntVector2, FGridData>& FHexGridMap::GetGridInfo() const
{ 
    return GridInfo;
}
