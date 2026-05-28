// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/CombatManagerComponent.h"

#include "Combat/Grid/Arena.h"
#include "Combat/Grid/ArenaManager.h"
#include "Pawn/NGEnemyPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Components/NGPocketComponent.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Pawn/NGSpectatorPawn.h"
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

void UCombatManagerComponent::StartCombat(FCombatSettingData& SettingData, APlayerController* PC)
{
	if (!GetOwner()->HasAuthority())	return;
	
	SetupCombat(SettingData);

	//화면띄우고 이것저것
	
	//TODO: 모든 플레이어가 Rep된거 확인하고 타이머시작이 더 좋을듯 -> GS에서 PS순회해서 nullptr없어야 고?
	GetWorld()->GetTimerManager().SetTimer(FightStartTimerHandle, this, &UCombatManagerComponent::StartFight, 0.1f, false);	
}

void UCombatManagerComponent::StartFight()
{
	if (!GetOwner()->HasAuthority())	return;
	
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return;
	
	//모든 유저 경기장 순회하면서 전투상태로 변경
	for (APlayerState* RawPlayer : GS->PlayerArray)
	{
		if (IsValid(RawPlayer))
		{
			if (ANGPlayerState* Player = Cast<ANGPlayerState>(RawPlayer))
			{
				Player->PrepareStartCombat();
			}
		}
	}
}

void UCombatManagerComponent::ReturnSpectatorHome()
{
	for (FCombatSettingData Data : CombatDatas)
	{
		if (ANGPlayerState* AwayPlayer = Data.PlayerB)
		{
			if (AArenaManager* ArenaManager = AwayPlayer->GetArenaManager())
			{
				FArenaAddress HomeArena(AwayPlayer->GetHomeArena(), ::EPossessArenaIdentification::Home);
				ArenaManager->PossessArena(HomeArena);
			}

		}
	}
}

void UCombatManagerComponent::FinishCombat()
{
	if (!GetOwner()->HasAuthority())	return;
	
	FCombatResultData CombatResult = {};
	
	TransitionCombatPlayerGameStates(EGameState::Exploration);
	
	//Grid원래상태로 초기화
	ResetGrid();
	
	//카메라 위치 원래자리로
	ReturnSpectatorHome();
	
	if (ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>())
	{
		GM->OnCombatFinished(CombatResult);
	}
	
	CombatDatas.Empty();
}

void UCombatManagerComponent::TransitionCombatPlayerGameStates(EGameState GameState)
{
	for (FCombatSettingData Data : CombatDatas)
	{
		if (Data.PlayerA)
		{
			Data.PlayerA->SetGameState(GameState);
		}
		
		if (Data.PlayerB)
		{
			Data.PlayerB->SetGameState(GameState);
		}
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

void UCombatManagerComponent::SetupCombat(FCombatSettingData& SettingData)
{
	if (!SettingData.PlayerA)	return;
	if (!SettingData.PlayerB)	return;
	
	//전투 시작 전 그리드 상태 복구용 스냅샷
	
	//TODO: SettingData를 Array로 바꿔서 순환하면서 ㄱㄱ
	
	SettingData.PlayerA->CaptureSnapShot();
	SettingData.PlayerA->SetGameState(EGameState::Combat);

	SettingData.PlayerB->CaptureSnapShot();
	SettingData.PlayerB->SetGameState(EGameState::Combat);
	
	CurrentEnemyCount = 0;
	TargetKillCount = SettingData.EnemyCount;
	
	if (AArenaManager* ArenaManager = SettingData.PlayerB->GetArenaManager())
	{
		FArenaAddress AwayArenaAddress(SettingData.PlayerA->GetHomeArena(), EPossessArenaIdentification::Away);
		ArenaManager->PossessArena(AwayArenaAddress);
	}
	
	CombatDatas.Emplace(MoveTemp(SettingData));
}

void UCombatManagerComponent::ResetGrid()
{
	if (!GetOwner()->HasAuthority())	return;
	
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (GS)
	{
		for (APlayerState* RawPS : GS->PlayerArray)
		{
			//TODO: 현재 플레이어가 살아있는 상태에서만 Restore
			if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
			{
				PS->RestoreInitialGrid();
			}
		}
	}	
	
}