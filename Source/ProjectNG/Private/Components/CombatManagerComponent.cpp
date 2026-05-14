// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/CombatManagerComponent.h"

#include "Pawn/NGEnemyPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Components/NGPocketComponent.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Player/NGPlayerState.h"

// Sets default values
UCombatManagerComponent::UCombatManagerComponent()
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

	//화면띄우고 이것저것
	
	//TODO: 모든 플레이어가 Rep된거 확인하고 타이머시작이 더 좋을듯
	GetWorld()->GetTimerManager().SetTimer(FightStartTimerHandle, this, &UCombatManagerComponent::StartFight, 0.1f, false);	
}

void UCombatManagerComponent::StartFight()
{
	if (!GetOwner()->HasAuthority())	return;
	
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return;
	
	//모든 유저 경기장 순회하면서 전투상태로 변경
	for (ANGPlayerState* Player : GS->PlayerStates)
	{
		if (IsValid(Player))
		{
			Player->PrepareStartCombat();
		}
	}
}

void UCombatManagerComponent::FinishCombat()
{
	FCombatResultData CombatResult = {};
	
	//Grid원래상태로 초기화
	ResetGrid();
	
	//결과창 띄우기하고 확인 후 값 반환해야할 수 있음
	//게임 전체 매니저에 FCombatResultData를 주는 식으로 함수호출을 해야겠는데?
	if (ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>())
	{
		GM->OnCombatFinished(CombatResult);
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
	
	if (SettingData.PlayerA)
	{
		HomePS = SettingData.PlayerA;
	}
	
	if (SettingData.PlayerB)
	{
		AwayPS = SettingData.PlayerB;
		
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


void UCombatManagerComponent::ResetGrid()
{
	if (!GetOwner()->HasAuthority())	return;
	
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (GS)
	{
		for (ANGPlayerState* PS : GS->PlayerStates)
		{
			PS->RestoreInitialGrid();
		}
	}	
	
}