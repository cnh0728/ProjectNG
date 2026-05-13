// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/CombatManagerComponent.h"

#include "Pawn/NGEnemyPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Components/NGPocketComponent.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerState.h"

// Sets default values
UCombatManagerComponent::UCombatManagerComponent() : CurrentWaveIndex(0), EnemiesSpawnedSoFar(0)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts or when spawned
void UCombatManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatManagerComponent::StartCombat(FCombatSettingData SettingData, APlayerController* PC)
{
	if (!GetOwner()->HasAuthority())	return;
	
	SetupCombat(SettingData);
	
	// StartWave(PC);
	//화면띄우고 이것저것
	
}

void UCombatManagerComponent::FinishCombat()
{
	FCombatResultData CombatResult = {};
	
	//결과창 띄우기하고 확인 후 값 반환해야할 수 있음
	
	//게임 전체 매니저에 FCombatResultData를 주는 식으로 함수호출을 해야겠는데?
	if (ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>())
	{
		GM->OnCombatFinished(CombatResult);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Find GameMode"));
	}
}

void UCombatManagerComponent::PawnDied(ANGPawnBase* DeadPawn)
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

void UCombatManagerComponent::SetupCombat(FCombatSettingData SettingData)
{
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return;
	
	//전투 시작 전 그리드 상태 복구용 스냅샷
	for (ANGPlayerState* PlayerState : GS->PlayerStates)
	{
		if (PlayerState)
		{
			PlayerState->CaptureSnapShot();
		}
	}
	
	CurrentEnemyCount = 0;
	TargetKillCount = SettingData.EnemyCount;
	
	if (SettingData.PlayerB)
	{
		UNGPocketComponent* PocketComponentB = SettingData.PlayerB->GetPlayerPocket();
		for (TWeakObjectPtr<ANGUnitPawn> Unit : PocketComponentB->GetOwnedUnitPocket())
		{
			FGridAddress GridAddress = Unit->GetGridAddress();
			FIntVector2 MirroredIdx = UGridMapHelper::GetMirroredIndex(*UGridMapHelper::GetGridMap(GridAddress), GridAddress.GridIndex);
			
			FGridAddress EnemyCombatGridAddress(MirroredIdx, EGridType::EnemyWait, SettingData.PlayerA);
			if (GridAddress.GridType == EGridType::Combat)
			{
				EnemyCombatGridAddress.GridType = EGridType::Combat;
			}
			Unit->SetPawnOnGrid(EnemyCombatGridAddress);
		}
	}
}

void UCombatManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatManagerComponent, CurrentWaveIndex);
}
