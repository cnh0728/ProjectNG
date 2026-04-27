// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/GridMapManager.h"

#include "Combat/Grid/Grid.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Core/NGDeveloperSettings.h"
#include "Core/NGPoolSubSystem.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameGameMode.h"
#include "ProjectNG/ProjectNG.h"

class UNGDeveloperSettings;

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

bool AGridMapManager::IsPossibleSpawnPawn(AGridMapManager* MapManager) const
{
	TOptional<FIntVector2> EmptyGridIndex = MapManager->GridMap.GetEmptyGridIndex();
	
	if (!EmptyGridIndex.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid is full"));
		return false;
	}
	
	return true;
}

bool AGridMapManager::SpawnUnitPawn(FName UnitName, APlayerController* RequestingPlayer) const
{
	if (!HasAuthority())	return false;
	
	//여기서부터 아래가 소환로직
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return false;
	
	AGridMapManager* MapManager = GS->GetGridMapManager();
	if (!MapManager)	return false;
	
	UNGPoolSubSystem* Pool = GetWorld()->GetSubsystem<UNGPoolSubSystem>();
	if (!Pool)	return false;
	
	TOptional<FIntVector2> EmptyGridIndex = MapManager->GridMap.GetEmptyGridIndex();
		
	if (!IsPossibleSpawnPawn(MapManager))
	{
		return false;
	}
	
	UDataTable* UnitDataTable = GS->GetUnitDataTable();
	
	if (UnitDataTable)
	{
		FUnitData* FoundRow = UnitDataTable->FindRow<FUnitData>(UnitName, TEXT(""));

		if (FoundRow && FoundRow->UnitClass)
		{
			FVector SpawnLocation = MapManager->GridMap.GetWorldLocation(EmptyGridIndex.GetValue());
						
			ANGUnitPawn* DefaultUnit = FoundRow->UnitClass->GetDefaultObject<ANGUnitPawn>();
					
			if (DefaultUnit)
			{
				FVector HalfHeight = DefaultUnit->GetHalfCapsule();
							
				SpawnLocation += HalfHeight;
			}
						
			FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
			// UClass* CC = GetDefault<UNGDeveloperSettings>()->PawnClass[FoundRow->UnitClass].LoadSynchronous();
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = RequestingPlayer;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			ANGUnitPawn* NewPawn = Cast<ANGUnitPawn>(Pool->AcquirePawn(FoundRow->UnitClass, SpawnTransform, SpawnParams));
						
			if (!NewPawn)
			{
				UE_LOG(LogTemp, Warning, TEXT("NewPawn is nullptr"));
				return false;
			}
						
			UCapsuleComponent* Capsule = NewPawn->GetCapsuleComponent();
			if (Capsule)
			{
				Capsule->SetCollisionResponseToChannel(ECC_SelectableUnit, ECR_Block);
				Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
						
			//여기서 찾은 그리드에 값 기입
			FGridData GridData;
			GridData.PlacedPawn = NewPawn;
			
			NewPawn->SetPlacedGridIndex(EmptyGridIndex.GetValue());
			MapManager->GridMap.SetGridData(EmptyGridIndex.GetValue(), GridData);
						
			return true;
		}
	}
	return false;
}

void AGridMapManager::BeginPlay()
{
	Super::BeginPlay();

	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		GS->InitializeGridMapManager(this);
	}
	
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

	// 1. 모든 타일의 월드 좌표를 뒤져서 최소/최대 경계를 찾습니다.
	float MinX = FLT_MAX, MaxX = -FLT_MAX, MinY = FLT_MAX, MaxY = -FLT_MAX;

	for (const auto& Pair : GridMap.GetGridInfo()) // GridInfo나 EmptyGridIndex 순회
	{
		FVector Loc = GridMap.GetWorldLocation(Pair.Key);
		MinX = FMath::Min(MinX, Loc.X);
		MaxX = FMath::Max(MaxX, Loc.X);
		MinY = FMath::Min(MinY, Loc.Y);
		MaxY = FMath::Max(MaxY, Loc.Y);
	}

	// 2. 여유 공간(PushDistance)을 둔 4개의 직사각형 모서리 좌표 설정
	TArray<FVector> Points;
	Points.Add(FVector(MinX - PushDistance, MinY - PushDistance, 0.f)); // 좌하
	Points.Add(FVector(MaxX + PushDistance, MinY - PushDistance, 0.f)); // 우하
	Points.Add(FVector(MaxX + PushDistance, MaxY + PushDistance, 0.f)); // 우상
	Points.Add(FVector(MinX - PushDistance, MaxY + PushDistance, 0.f)); // 좌상

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
