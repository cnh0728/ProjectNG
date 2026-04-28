// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/GridMapManager.h"

#include "Combat/Grid/Grid.h"
#include "Components/NGPocketComponent.h"
#include "Components/SplineComponent.h"
#include "Core/NGSpawnHelper.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"

AGridMapManager::AGridMapManager()
{
	PrimaryActorTick.bCanEverTick = false;
	
	//UPROPERTY로 값 수정하게 하고 싶으면 생성자말고 다른데로 옮겨야할수도 ㅇㅇ
	InitGridMap(5, 5, 100.f);
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	EnemyPathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("EnemyPathSpline"));
	EnemyPathSpline->SetupAttachment(Root);
	//InitGridMap뒤에 선언할 것 (GridMap기준으로 만듦)
	MakeEnemySpline();
}

bool AGridMapManager::IsPossibleSpawnPawn() const
{
	TOptional<FIntVector2> EmptyGridIndex = GridMap.GetEmptyGridIndex();
	
	if (!EmptyGridIndex.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid is full"));
		return false;
	}
	
	return true;
}

bool AGridMapManager::SpawnUnitPawn(FName UnitName, APlayerController* RequestingPlayer)
{
	TOptional<FIntVector2> EmptyGridIndex = GridMap.GetEmptyGridIndex();
		
	if (!IsPossibleSpawnPawn())		return false;
	
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return false;
	
	UDataTable* UnitDataTable = GS->GetUnitDataTable();
	if (!UnitDataTable)	return false;
	
	FUnitData* FoundRow = UnitDataTable->FindRow<FUnitData>(UnitName, TEXT(""));
	if (!FoundRow || !FoundRow->UnitClass)	return false;
	
	FVector SpawnLocation = GridMap.GetWorldLocation(EmptyGridIndex.GetValue());
	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	ANGUnitPawn* NewPawn = UNGSpawnHelper::SpawnPawn<ANGUnitPawn>(this, FoundRow->UnitClass, SpawnTransform, RequestingPlayer);
	if (!NewPawn)	return false;
	
	//여기서 찾은 그리드에 값 기입
	FGridData GridData;
	GridData.PlacedPawn = NewPawn;
			
	NewPawn->SetPlacedGridIndex(EmptyGridIndex.GetValue());
	GridMap.SetGridData(EmptyGridIndex.GetValue(), GridData);

	if (ANGPlayerState* PS = RequestingPlayer->GetPlayerState<ANGPlayerState>())
	{
		if (UNGPocketComponent* Pocket = PS->GetPlayerPocket())
		{
			Pocket->AddUnit(NewPawn);
			//기본적으로 Wait에 넣어야함, Wait에서 전투로 옮기는건 
			Pocket->AddWaitUnit(NewPawn);
		}
	}
	
	return true;
}

void AGridMapManager::BeginPlay()
{
	Super::BeginPlay();
	
	GridMap.ResetEmptyGridIndex();
}

void AGridMapManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AGridMapManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	DrawGridLine();
}

void AGridMapManager::MakeEnemySpline()
{
	if (GridMap.CellSize <= 0.f || !IsValid(EnemyPathSpline)) return;
    
	EnemyPathSpline->ClearSplinePoints(true);
	float PushDistance = GridMap.CellSize + SplineMarginFromEdge;

	FVector MinCoor = GridMap.GetWorldLocation(FIntVector2(0, 0));
	FVector MaxCoor = GridMap.GetWorldLocation(FIntVector2(GridMap.CountQ - 1, GridMap.CountR - 1));

	// 2. 여유 공간(PushDistance)을 둔 4개의 직사각형 모서리 좌표 설정
	TArray<FVector> Points;
	Points.Add(FVector(MinCoor.X - PushDistance, MinCoor.Y - PushDistance, 0.f)); // 좌하
	Points.Add(FVector(MaxCoor.X + PushDistance, MinCoor.Y - PushDistance, 0.f)); // 우하
	Points.Add(FVector(MaxCoor.X + PushDistance, MinCoor.Y + PushDistance, 0.f)); // 우상
	Points.Add(FVector(MaxCoor.X - PushDistance, MinCoor.Y + PushDistance, 0.f)); // 좌상

	for (int32 i = 0; i < Points.Num(); ++i)
	{
		EnemyPathSpline->AddSplinePoint(Points[i], ESplineCoordinateSpace::Local, false);
		EnemyPathSpline->SetSplinePointType(i, ESplinePointType::Linear, false);
	}
    
	EnemyPathSpline->UpdateSpline();
	EnemyPathSpline->SetClosedLoop(true);
}

void AGridMapManager::DrawGridLine()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FlushPersistentDebugLines(World);
	GridMap.Pivot = GetActorLocation();

	const int32 SizeQ = GridMap.CountQ;
	const int32 SizeR = GridMap.CountR;

	// 모든 타일을 순회하며 육각형 그리기
	for (int32 q = 0; q < SizeQ; ++q)
	{
		for (int32 r = 0; r < SizeR; ++r)
		{
			FIntVector2 CurrentIndex(q, r);
			FVector Center = GridMap.GetWorldLocation(CurrentIndex);

			// 육각형의 6개 꼭짓점을 계산해서 선으로 연결
			for (int32 i = 0; i < 6; ++i)
			{
				FVector StartCorner = Center + GridMap.GetHexCorner(i);
				FVector EndCorner = Center + GridMap.GetHexCorner(i + 1);

				DrawDebugLine(World, StartCorner, EndCorner, GridLineColor, true, -1.0f, 0, LineThickness);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Hex GridMap is Visualized."));
}

void AGridMapManager::InitGridMap(const int32 InitSizeX, const int32 InitSizeY, const double CellSize)
{
	const FVector MyLocation = GetActorLocation();
	
	GridMap.InitializeMap(InitSizeX, InitSizeY, CellSize, MyLocation);
	GridMap.ResetGridInfo();

}
