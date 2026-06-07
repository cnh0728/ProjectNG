// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Components/NGCombatManagerComponent.h"
#include "Components/NGPocketComponent.h"
#include "Combat/Grid/ArenaManager.h"
#include "Pawn/NGEnemyPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"

#include "Algo/RandomShuffle.h"

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

void UNGCombatManagerComponent::NotifyEndCombat(const ANGPlayerState* LoseEndPlayer)
{
	//폰이 죽었을때 자기가 졌는지 확인하고 호출하는애라 CombatDatas에서 찾아서 반대편에 애는 승리처리
	if (!GetOwner()->HasAuthority())	return;

	UE_LOG(LogTemp, Log, TEXT("NotifyEndCombat"));
	
	int32 FoundDataIndex = INDEX_NONE;
	int32 LoserPlayerIndex = INDEX_NONE;
	
	for (int32 i = 0; i < CombatDatas.Num(); ++i)
	{
		if (CombatDatas[i].Players[0] == LoseEndPlayer) { FoundDataIndex = i; LoserPlayerIndex = 0; break; }
		if (CombatDatas[i].Players[1] == LoseEndPlayer) { FoundDataIndex = i; LoserPlayerIndex = 1; break; }
	}
	
	bool bIsNewNotification = false;
	
	if (FoundDataIndex != INDEX_NONE)
	{
		int32 WinnerPlayerIndex = 1 - LoserPlayerIndex;
		const FCombatSettingData& TargetCombat = CombatDatas[FoundDataIndex];
		
		if (ANGPlayerState* Winner = TargetCombat.Players[WinnerPlayerIndex].Get())
		{
			if (CombatResultDictionary.Find(Winner) == nullptr)
			{
				CombatResultDictionary.Add(Winner, ECombatResult::Win);
				bIsNewNotification = true;
			}else
			{
				uint8 WinCombinedResult = static_cast<uint8>(CombatResultDictionary[Winner]) | static_cast<uint8>(ECombatResult::Win);
				CombatResultDictionary[Winner] = static_cast<ECombatResult>(WinCombinedResult);
			}
		}
		
		if (ANGPlayerState* Loser = TargetCombat.Players[LoserPlayerIndex].Get())
		{
			if (CombatResultDictionary.Find(Loser) == nullptr)
			{
				CombatResultDictionary.Add(Loser, ECombatResult::Lose);
				bIsNewNotification = true;
			}else
			{
				uint8 LoseCombinedResult = static_cast<uint8>(CombatResultDictionary[Loser]) | static_cast<uint8>(ECombatResult::Lose);
				CombatResultDictionary[Loser] = static_cast<ECombatResult>(LoseCombinedResult);
			}
		}
	}
	
	if (bIsNewNotification)
	{
		//진곳에서만 호출하니까 CombatDatas 수랑 같이 올라가서 동기화됨
		if (++FinishedCombatCount >= CombatDatas.Num())
		{
			FinishCombat();
		}
	}
}

void UNGCombatManagerComponent::FinishCombat()
{
	if (!GetOwner()->HasAuthority())	return;
	
	FCombatResultData CombatResult = {};
	
	EGameState EndGameState = EGameState::Exploration;
	
	for (FCombatSettingData Data : CombatDatas)
	{
		if (Data.Players.Num() > 0)
		{
			if (ANGPlayerState* HomePlayer = Data.Players[0].Get())
			{
				// HomePlayer->OnCombatEnd(CombatWinDictionary[HomePlayer]);
				HomePlayer->SetGameState(EndGameState);
				
				ResetGrid(HomePlayer);
			}
		}
		
		if (Data.Players.Num() > 1)
		{
			if (ANGPlayerState* AwayPlayer = Data.Players[1].Get())
			{
				// AwayPlayer->OnCombatEnd(CombatWinDictionary[AwayPlayer]);
				AwayPlayer->SetGameState(EndGameState);

				ResetGrid(AwayPlayer);
				ReturnSpectatorHome(AwayPlayer);
			}
		}
	}
	
	if (ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>())
	{
		GM->OnCombatFinished(CombatResult);
	}

	ClearCombatEndDatas();
}

void UNGCombatManagerComponent::ClearCombatEndDatas()
{
	//CombatWinDictionary는 굳이 초기화할 필요 없어서 안함
	FinishedCombatCount = 0;
	CombatResultDictionary.Reset();
	CombatDatas.Empty();
}

void UNGCombatManagerComponent::NotifyPawnDied(ANGPawnBase* DeadPawn)
{
	if (!GetOwner()->HasAuthority())	return;
	
	if (!DeadPawn)	return;
	
	//TODO: Enum으로 Type 넣어두던가 인터페이스 형태로? 바꾸는게 나을듯
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
	
	ANGPlayerController* OwnerPC = DeadPawn->GetOwner<ANGPlayerController>();
	ANGPlayerState* OwnerPS = OwnerPC ? OwnerPC->GetPlayerState<ANGPlayerState>() : nullptr;
	if (UNGPocketComponent* Pocket = OwnerPS ? OwnerPS->GetPlayerPocket() : nullptr)
	{
		if (Pocket->IsAnnihilated())
		{
			NotifyEndCombat(OwnerPS);
		}
	}
	
}

void UNGCombatManagerComponent::SetupCombat(const FCombatSettingData& SettingData)
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