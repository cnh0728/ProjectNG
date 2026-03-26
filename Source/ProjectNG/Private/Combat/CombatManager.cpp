// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/CombatManager.h"

#include "Character/NGEnemyCharacter.h"
#include "Character/NGUnitCharacter.h"
#include "Combat/GridMapManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "GameModes/NGInGameGameMode.h"
#include "Net/UnrealNetwork.h"


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
	
	AGridMapManager* GridMapManager = nullptr;
	
	if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>())
	{
		GridMapManager = GM->GetGridMapManager();
	}

	if (!IsValid(GridMapManager))
	{
		return;
	}
		
	if (!IsValid(GridMapManager->EnemyPathSpline))	return;
	
	UE_LOG(LogTemp, Warning, TEXT("Enemy Spawned!"));
	
	FVector SpawnLocation = GridMapManager->EnemyPathSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	
	FRotator SpawnRotation = GridMapManager->EnemyPathSpline->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);
	
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
		NewEnemy->InitPatrolPath(GridMapManager->EnemyPathSpline, CapsuleHalfHeight);
	}
	
	EnemiesSpawnedSoFar++;
}


void ACombatManager::StartCombat(FCombatSettingData SettingData)
{
	SetupCombat(SettingData);
	
	StartWave();
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

void ACombatManager::CharacterDied(ANGCharacterBase* DeadCharacter)
{
	if (!DeadCharacter)	return;
	
	if (DeadCharacter->IsA(ANGEnemyCharacter::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("적 사망"));
		++CurrentEnemyCount;
		//적 사망 이벤트
		//적 사망 델리게이트 만들어서 구독시키게 하고 델리게이트 호출도 나쁘지 않을듯
	}else if (DeadCharacter->IsA(ANGUnitCharacter::StaticClass()))
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
	CurrentEnemyCount = 0;
	TargetKillCount = SettingData.EnemyCount;
}

void ACombatManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACombatManager, CurrentWaveIndex);
}
