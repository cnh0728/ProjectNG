// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/GridMapManager.h"

#include "Combat/Grid/Grid.h"
#include "Components/SplineComponent.h"
#include "Player/NGPlayerState.h"


AGridMapManager::AGridMapManager()
{
	bReplicates = true;
	
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	EnemyPathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("EnemyPathSpline"));
	EnemyPathSpline->SetupAttachment(Root);
}

void AGridMapManager::Initialize(ANGPlayerController* InPC)
{
	OwnerPCCache = InPC;
	
	InitGridMap(8, 8, 100.f);
	
	MakeEnemySpline();
	
	if (ANGPlayerState* PS = OwnerPCCache->GetPlayerState<ANGPlayerState>())
	{
		PS->GridMap.ResetEmptyGridIndex();
	}
}

void AGridMapManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	DrawGridLine();
}

void AGridMapManager::BeginPlay()
{
	Super::BeginPlay();
	
	SetNetDormancy(DORM_Initial);
}

void AGridMapManager::MakeEnemySpline()
{
	if (!OwnerPCCache)	return;
	
	ANGPlayerState* PS = OwnerPCCache->GetPlayerState<ANGPlayerState>();
	if (!PS) return;

	if (PS->GridMap.CellSize <= 0.f || !IsValid(EnemyPathSpline)) return;
    
	EnemyPathSpline->ClearSplinePoints(true);
	float PushDistance = PS->GridMap.CellSize + SplineMarginFromEdge;

	FVector MinCoor = PS->GridMap.GetWorldLocation(FIntVector2(0, 0));
	FVector MaxCoor = PS->GridMap.GetWorldLocation(FIntVector2(PS->GridMap.CountQ - 1, PS->GridMap.CountR - 1));

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

	if (!OwnerPCCache)	return;
	
	ANGPlayerState* PS = OwnerPCCache->GetPlayerState<ANGPlayerState>();
	if (!PS) return;
	
	FlushPersistentDebugLines(World);
	PS->GridMap.Pivot = GetActorLocation();

	const int32 SizeQ = PS->GridMap.CountQ;
	const int32 SizeR = PS->GridMap.CountR;

	// 모든 타일을 순회하며 육각형 그리기
	for (int32 q = 0; q < SizeQ; ++q)
	{
		for (int32 r = 0; r < SizeR; ++r)
		{
			FIntVector2 CurrentIndex(q, r);
			FVector Center = PS->GridMap.GetWorldLocation(CurrentIndex);

			// 육각형의 6개 꼭짓점을 계산해서 선으로 연결
			for (int32 i = 0; i < 6; ++i)
			{
				FVector StartCorner = Center + PS->GridMap.GetHexCorner(i);
				FVector EndCorner = Center + PS->GridMap.GetHexCorner(i + 1);

				DrawDebugLine(World, StartCorner, EndCorner, GridLineColor, true, -1.0f, 0, LineThickness);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Hex OwnerPSCache->GridMap is Visualized."));
}

void AGridMapManager::InitGridMap(const int32 InitSizeX, const int32 InitSizeY, const double CellSize)
{
	const FVector MyLocation = GetActorLocation();
	
	if (OwnerPCCache)
	{
		if (ANGPlayerState* PS = OwnerPCCache->GetPlayerState<ANGPlayerState>())
		{
			PS->GridMap.InitializeMap(InitSizeX, InitSizeY, CellSize, MyLocation);
			PS->GridMap.ResetGridInfo();
		}
	}
}
