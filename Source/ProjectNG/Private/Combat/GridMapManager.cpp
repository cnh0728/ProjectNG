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

bool AGridMapManager::IsPossibleSpawnCharacter(AGridMapManager* MapManager) const
{
	TOptional<FIntVector2> EmptyGridIndex = MapManager->GridMap.GetEmptyGridIndex();
	
	if (!EmptyGridIndex.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid is full"));
		return false;
	}
	
	return true;
}

bool AGridMapManager::SpawnUnitCharacter(FName UnitName) const
{
	if (!HasAuthority())	return false;
	
	//여기서부터 아래가 소환로직
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return false;
	
	ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>();
	if (!GM)	return false;
	
	AGridMapManager* MapManager = GM->GetGridMapManager();
	if (!MapManager)	return false;
	
	UNGPoolSubSystem* Pool = GetWorld()->GetSubsystem<UNGPoolSubSystem>();
	if (!Pool)	return false;
	
	TOptional<FIntVector2> EmptyGridIndex = MapManager->GridMap.GetEmptyGridIndex();
		
	if (!IsPossibleSpawnCharacter(MapManager))
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
						
			ANGUnitCharacter* DefaultUnit = FoundRow->UnitClass->GetDefaultObject<ANGUnitCharacter>();
					
			if (DefaultUnit)
			{
				FVector HalfHeight = DefaultUnit->GetHalfCapsule();
							
				SpawnLocation += HalfHeight;
			}
						
			FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
			// UClass* CC = GetDefault<UNGDeveloperSettings>()->CharacterClass[FoundRow->UnitClass].LoadSynchronous();
			
			// ANGUnitCharacter* NewCharacter = Cast<ANGUnitCharacter>(Pool->AcquireCharacter(CC, SpawnTransform));
			ANGUnitCharacter* NewCharacter = Cast<ANGUnitCharacter>(Pool->AcquireCharacter(FoundRow->UnitClass, SpawnTransform));
						
			if (!NewCharacter)
			{
				UE_LOG(LogTemp, Warning, TEXT("NewCharacter is nullptr"));
				return false;
			}
						
			UCapsuleComponent* Capsule = NewCharacter->GetCapsuleComponent();
			if (Capsule)
			{
				Capsule->SetCollisionResponseToChannel(ECC_SelectableUnit, ECR_Block);
				Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
						
			//여기서 찾은 그리드에 값 기입
			FGridData GridData;
			GridData.PlacedCharacter = NewCharacter;
			
			NewCharacter->SetPlacedGridIndex(EmptyGridIndex.GetValue());
			MapManager->GridMap.SetGridData(EmptyGridIndex.GetValue(), GridData);
						
			return true;
		}
	}
	return false;
}

void AGridMapManager::BeginPlay()
{
	Super::BeginPlay();

	if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>())
	{
		GM->InitializeGridMapManager(this);
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
	if (GridMap.CellSize <= 0.f) return;
	
	if (!IsValid(EnemyPathSpline)) return;
	
	EnemyPathSpline->ClearSplinePoints(true);
	
	float PushDistance = GridMap.CellSize + SplineMarginFromEdge;
	
	//그리드 기준 각 모서리에서 0이면 -PushDistance, 최대치면 +PushDistance 해서 거기 스플라인 포인트 찍기
	
	for (int i=0;i<4;i++)
	{
		int32 UseTopX = i / 2;
		int32 UseTopY = ((i + 1) / 2) % 2;
		
		FVector CornerLocation = GridMap.GetRelativeLocation(FIntVector2((GridMap.CountX - 1) * UseTopX, (GridMap.CountY - 1) * UseTopY));
		FVector SplinePosition = CornerLocation + FVector(UseTopX ? PushDistance : -PushDistance, UseTopY? PushDistance : -PushDistance, 0.f);
		EnemyPathSpline->AddSplinePoint(SplinePosition, ESplineCoordinateSpace::Local, false);
		EnemyPathSpline->SetSplinePointType(i, ESplinePointType::Linear, false);
	}
	
	EnemyPathSpline->UpdateSpline();
	EnemyPathSpline->SetClosedLoop(true);
}

void AGridMapManager::DrawGridLine()
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	// 2. GridMap 데이터 가져오기 (UGridMap::GetSizeX(), UGridMap::GetCellSize() 같은 함수를 가정)
	// 실제 GridMap에 접근하는 코드는 FGridMap의 구조에 따라 다릅니다.
    
	const int32 SizeX = GridMap.CountX;
	const int32 SizeY = GridMap.CountY;
	const float TileSize = GridMap.CellSize;
	
	// 이전 디버그 라인 제거 (에디터 갱신 시 잔상 방지)
	FlushPersistentDebugLines(World);

	GridMap.Pivot = GetActorLocation();
	// 3. 디버그 라인 그리기 (OnConstruction에서 그릴 때는 'Persistent' 옵션을 사용해야 합니다)
	// (선 그리기 로직은 이전 답변과 동일)
    
	// 세로선 (X축 따라 이동하며 Y축 방향으로 긋기)
	for (int32 x = 0; x <= SizeX; ++x)
	{
		FVector LineStart = FVector(x * TileSize + GridMap.Pivot.X,  GridMap.Pivot.Y, 0);
		FVector LineEnd   = FVector(x * TileSize + GridMap.Pivot.X, SizeY * TileSize + GridMap.Pivot.Y, 0);
        
		// 5초 지속 (false), 두께 2.0f
		DrawDebugLine(World, LineStart, LineEnd, GridLineColor, true, -1.0f, 0, LineThickness);
	}

	// 가로선 (Y축 따라 이동하며 X축 방향으로 긋기)
	for (int32 y = 0; y <= SizeY; ++y)
	{
		FVector LineStart = FVector(GridMap.Pivot.X, y * TileSize + GridMap.Pivot.Y, 0);
		FVector LineEnd   = FVector(SizeX * TileSize + GridMap.Pivot.X, y * TileSize + GridMap.Pivot.Y, 0);
        
		DrawDebugLine(World, LineStart, LineEnd, GridLineColor, true, -1.0f, 0, LineThickness);
	}

	UE_LOG(LogTemp, Warning, TEXT("GridMap is Visualized."));
}

void AGridMapManager::InitGridMap(const int32 InitSizeX, const int32 InitSizeY, const double CellSize)
{
	const FVector MyLocation = GetActorLocation();
	
	GridMap.InitializeMap(InitSizeX, InitSizeY, CellSize, MyLocation);
	GridMap.ResetGridInfo();

}
