// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Algo/RandomShuffle.h"

#include "Combat/Grid/ArenaManager.h"
#include "Components/NGCombatManagerComponent.h"
#include "Pawn/NGEnemyPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Player/NGPlayerState.h"

// Sets default values
UNGCombatManagerComponent::UNGCombatManagerComponent() : FightWaitTime(0.1f)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// Called when the game starts or when spawned
void UNGCombatManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNGCombatManagerComponent::EnqueueCombatPhase(ANGPlayerState* PS)
{
	if (!PS)	return;
	
	CombatPSQueue.Add(PS);
}

void UNGCombatManagerComponent::StartCombat()
{
	Algo::RandomShuffle(CombatPSQueue);
	
	uint8 Checker = 0;
	
	FCombatSettingData CombatSettingData;
	
	for (ANGPlayerState* PS : CombatPSQueue)
	{
		CombatSettingData.Players[Checker++] = PS;
		
		if (Checker > 1)
		{
			Checker = 0;
			SetupCombat(CombatSettingData);
			
			CombatSettingData.Reset();
		}
	}
	//TODO: 홀수일때 처리필요 (부전승)
	
	GetWorld()->GetTimerManager().SetTimer(FightStartTimerHandle, this, &UNGCombatManagerComponent::StartFight, FightWaitTime, false);	
}

void UNGCombatManagerComponent::StartFight()
{
	if (!GetOwner()->HasAuthority())	return;
	
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return;
	
	UE_LOG(LogTemp, Log, TEXT("CombatDatas Num : %d"), CombatDatas.Num());
	
	//모든 유저 경기장 순회하면서 전투상태로 변경
	for (FCombatSettingData Data : CombatDatas)
	{
		for (ANGPlayerState* PS : Data.Players)
		{
			PS->PrepareStartCombat();
		}
	}
}

void UNGCombatManagerComponent::ReturnSpectatorHome(ANGPlayerState* AwayPlayer)
{
	if (AArenaManager* ArenaManager = AwayPlayer->GetArenaManager())
	{
		FArenaAddress HomeArena(AwayPlayer->GetHomeArena(), ::EPossessArenaIdentification::Home);
		ArenaManager->PossessArena(HomeArena);
	}
}

void UNGCombatManagerComponent::FinishCombat()
{
	if (!GetOwner()->HasAuthority())	return;
	
	FCombatResultData CombatResult = {};
	
	TransitionCombatPlayerGameStates(EGameState::Exploration);
	
	for (FCombatSettingData Data : CombatDatas)
	{
		if (Data.Players.Num() > 0)
		{
			ResetGrid(Data.Players[0]);
		}
		
		if (Data.Players.Num() > 1)
		{
			ResetGrid(Data.Players[1]);
			ReturnSpectatorHome(Data.Players[1]);
		}
	}
	
	//카메라 위치 원래자리로
	
	if (ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>())
	{
		GM->OnCombatFinished(CombatResult);
	}
	
	CombatDatas.Empty();
}

void UNGCombatManagerComponent::TransitionCombatPlayerGameStates(EGameState GameState)
{
	for (FCombatSettingData Data : CombatDatas)
	{
		if (Data.Players.Num() > 0)
		{
			Data.Players[0]->SetGameState(GameState);
		}
		
		if (Data.Players.Num() > 1)
		{
			Data.Players[1]->SetGameState(GameState);
		}
	}
}

void UNGCombatManagerComponent::PawnDied(ANGPawnBase* DeadPawn)
{
	if (!DeadPawn)	return;
	
	if (DeadPawn->IsA(ANGEnemyPawn::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("적 사망"));
		
		//TODO: 사망할때마다 사망한 Pocket체크해서 아무것도 없는지 확인
		//적 사망 이벤트
		//적 사망 델리게이트 만들어서 구독시키게 하고 델리게이트 호출도 나쁘지 않을듯
	}else if (DeadPawn->IsA(ANGUnitPawn::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("유닛 사망"));
		//유닛 죽었을때 이벤트
	}
	
	//TODO: 사망할때마다 사망한 Pocket체크해서 아무것도 없는지 확인
	// if (CheckPocket())
	{
		FinishCombat();
	}
}

void UNGCombatManagerComponent::SetupCombat(FCombatSettingData& SettingData)
{
	if (!SettingData.Players[0])	return;
	if (!SettingData.Players[1])	return;
	
	//전투 시작 전 그리드 상태 복구용 스냅샷
	for (ANGPlayerState* Player : SettingData.Players)
	{
		Player->CaptureSnapShot();
		Player->SetGameState(EGameState::Combat);
	}
	
	if (AArenaManager* ArenaManager = SettingData.Players[1]->GetArenaManager())
	{
		FArenaAddress AwayArenaAddress(SettingData.Players[0]->GetHomeArena(), EPossessArenaIdentification::Away);
		ArenaManager->PossessArena(AwayArenaAddress);
	}
	
	CombatDatas.Add(SettingData);
}

void UNGCombatManagerComponent::ResetGrid(ANGPlayerState* PS)
{
	if (!GetOwner()->HasAuthority())	return;
	
	if (PS)
	{
		PS->RestoreInitialGrid();
	}
}