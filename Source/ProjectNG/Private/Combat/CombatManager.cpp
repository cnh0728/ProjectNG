// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/CombatManager.h"

#include "Pawn/NGEnemyPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Combat/GridMapManager.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGPoolSubSystem.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerState.h"


class UNGDeveloperSettings;
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
}

// Called every frame
void ACombatManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACombatManager::StartWave(APlayerController* PC)
{
	if (!HasAuthority())	return;
	
	RequestingPlayerControllerCache = PC;
	
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
	//PC가져와서 넘겨줘야할듯?
	SpawnEnemy();
	
	// 목표 다 채웠으면 정지
	if (EnemiesSpawnedSoFar >= WaveList[CurrentWaveIndex].EnemyCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

bool ACombatManager::SpawnEnemy()
{
	AGridMapManager* GridMapManager = nullptr;
	
	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		//임시로 0
		GridMapManager = GS->GetGridMapManager(0);
	}

	if (!IsValid(GridMapManager))	return false;
	
	UNGPoolSubSystem* Pool = GetWorld()->GetSubsystem<UNGPoolSubSystem>();
	
	if (!Pool)	return false;
	
	UE_LOG(LogTemp, Warning, TEXT("Enemy Spawned!"));
	
	
	// TSubclassOf<ANGEnemyPawn> EnemyClass = WaveList[CurrentWaveIndex].EnemyClass;
	TSubclassOf<ANGEnemyPawn> EnemyClass = ANGEnemyPawn::StaticClass(); //TODO: 웨이브시스템 구현하면 위에꺼로 바꾸기
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = RequestingPlayerControllerCache;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	// ANGEnemyPawn* NewEnemy = UNGSpawnHelper::SpawnPawn<ANGEnemyPawn>(this, EnemyClass, SpawnTransform, RequestingPlayerControllerCache);
	// if (!NewEnemy)
	// {
	// 	return false;
	// }
	
	EnemiesSpawnedSoFar++;
	
	return true;
}


void ACombatManager::StartCombat(FCombatSettingData SettingData, APlayerController* PC)
{
	SetupCombat(SettingData);
	
	StartWave(PC);
	//화면띄우고 이것저것
	
}

void ACombatManager::FinishCombat()
{
	FCombatResultData CombatResult = {};
	
	//결과창 띄우기하고 확인 후 값 반환해야할 수 있음
	
	//게임 전체 매니저에 FCombatResultData를 주는 식으로 함수호출을 해야겠는데?
	if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>())
	{
		GM->OnCombatFinished(CombatResult);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Find GameMode"));
	}
}

void ACombatManager::PawnDied(ANGPawnBase* DeadPawn)
{
	if (!DeadPawn)	return;
	
	if (DeadPawn->IsA(ANGEnemyPawn::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("적 사망"));
		++CurrentEnemyCount;
		//적 사망 이벤트
		//적 사망 델리게이트 만들어서 구독시키게 하고 델리게이트 호출도 나쁘지 않을듯
	}else if (DeadPawn->IsA(ANGUnitPawn::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("유닛 사망"));
		//유닛 죽었을때 이벤트
	}
	
	if (CurrentEnemyCount >= TargetKillCount)
	{
		FinishCombat();
	}
}

void ACombatManager::SetupCombat(FCombatSettingData SettingData)
{
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return;
	
	AGridMapManager* MapManager = GS->GetGridMapManager(0);
	if (!MapManager)	return;
	
	CurrentEnemyCount = 0;
	TargetKillCount = SettingData.EnemyCount;
	
	const FHexGridMap& FightGridMap = SettingData.PlayerA->GetCombatGridMap();
	
	if (SettingData.PlayerA)
	{
		UNGPocketComponent* PocketComponentA = SettingData.PlayerA->GetPlayerPocket();
		
		for (TWeakObjectPtr<ANGUnitPawn> Unit : PocketComponentA->GetPlacedUnitPocket())
		{
			FVector TargetLoc = FightGridMap.GetWorldLocation(Unit->GetPlacedGridIndex());
			Unit->SetActorLocation(TargetLoc);
			Unit->SetActorRotation(FRotator::ZeroRotator);
		}
		
	}
	
	if (SettingData.PlayerB)
	{
		UNGPocketComponent* PocketComponentB = SettingData.PlayerB->GetPlayerPocket();
		for (TWeakObjectPtr<ANGUnitPawn> Unit : PocketComponentB->GetPlacedUnitPocket())
		{
			FIntVector2 MirroredIdx = FightGridMap.GetMirroredIndex(Unit->GetPlacedGridIndex());
			FVector TargetLoc = FightGridMap.GetWorldLocation(MirroredIdx);
			
			Unit->SetActorLocation(TargetLoc);
			Unit->SetActorRotation(FRotator(0.f, 180.f, 0.f));
		}
	}
}

void ACombatManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACombatManager, CurrentWaveIndex);
}
