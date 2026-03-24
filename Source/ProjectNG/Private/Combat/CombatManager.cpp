// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/CombatManager.h"

#include "Character/NGEnemyCharacter.h"
#include "Character/NGUnitCharacter.h"
#include "Combat/GridMapManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"
#include "ProjectNG/ProjectNG.h"


// Sets default values
ACombatManager::ACombatManager() : CurrentWaveIndex(0), EnemiesSpawnedSoFar(0)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();
	
	
	//서버에서만 실행
	if (HasAuthority())
	{
		if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
		{
			GS->InitializeCombatManager(this);
		}
	}
}

// Called every frame
void ACombatManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACombatManager::StartWave()
{
	if (!HasAuthority())	return;
	
	if (WaveList.IsValidIndex(CurrentWaveIndex))
	{
		EnemiesSpawnedSoFar = 0;
		
		// 타이머 시작 (SpawnInterval 마다 SpawnEnemyTimerElapsed 호출)
		float Interval = WaveList[CurrentWaveIndex].SpawnInterval;
		
		GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &ACombatManager::SpawnEnemyTimerElapsed, Interval, true);
	}
}

void ACombatManager::SpawnEnemyTimerElapsed()
{
	SpawnEnemy();
	
	// 목표 다 채웠으면 정지
	if (EnemiesSpawnedSoFar >= WaveList[CurrentWaveIndex].EnemyCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void ACombatManager::SpawnEnemy()
{
	UpdateGridManagerCache();
	
	if (!IsValid(GridMapManagerCache) || !IsValid(GridMapManagerCache->EnemyPathSpline))	return;
	
	UE_LOG(LogTemp, Warning, TEXT("Enemy Spawned!"));
	
	FVector SpawnLocation = GridMapManagerCache->EnemyPathSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	
	FRotator SpawnRotation = GridMapManagerCache->EnemyPathSpline->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);
	
	TSubclassOf<ANGEnemyCharacter> EnemyClass = WaveList[CurrentWaveIndex].EnemyClass;
	
	/////////////////////////
	/// 캐릭터 스폰 오프셋이 발끝기준으로 소환되게 조정
	FVector CapsuleHalfHeight = FVector::ZeroVector;
	if (IsValid(EnemyClass))
	{
		if (ACharacter* DefaultChar = Cast<ACharacter>(EnemyClass->GetDefaultObject()))
		{
			CapsuleHalfHeight.Z = DefaultChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			SpawnLocation += CapsuleHalfHeight;
		}
	}
	/////////////////////////
	
	if (ANGEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<ANGEnemyCharacter>(EnemyClass, SpawnLocation, SpawnRotation))
	{
		NewEnemy->InitPatrolPath(GridMapManagerCache->EnemyPathSpline, CapsuleHalfHeight);
	}
	
	EnemiesSpawnedSoFar++;
}

void ACombatManager::UpdateGridManagerCache()
{
	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		if (!IsValid(GridMapManagerCache))
		{
			GridMapManagerCache = GS->GetGridMapManager();
		}
	}
}

bool ACombatManager::IsPossibleSpawnCharacter(AGridMapManager* MapManager) const
{
	TOptional<FIntVector2> EmptyGridIndex = MapManager->GridMap.GetEmptyGridIndex();
	
	if (!EmptyGridIndex.IsSet())
	{
		UE_LOG(LogTemp, Warning, TEXT("Grid is full"));
		return false;
	}
	
	return true;
}

bool ACombatManager::SpawnUnitCharacter(FName UnitName) const
{
	//여기서부터 아래가 소환로직
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return false;
	
	AGridMapManager* MapManager = GS->GetGridMapManager();
	if (!MapManager)	return false;
		
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
						
			ANGUnitCharacter* NewCharacter = GetWorld()->SpawnActor<ANGUnitCharacter>(FoundRow->UnitClass, SpawnLocation, FRotator::ZeroRotator);
						
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

void ACombatManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACombatManager, CurrentWaveIndex);
}
