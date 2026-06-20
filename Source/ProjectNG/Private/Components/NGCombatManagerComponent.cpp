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
#include "Core/NGSpawnHelper.h"

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

void UNGCombatManagerComponent::EnqueueCombatPhase(ANGPlayerState* PS, const FEnemySquadData* CPUCombatData)
{
	if (!PS)	return;

	if (CPUCombatData)
	{
		FCombatSettingData CPUCombatSettingData;
		CPUCombatSettingData.bIsCPUCombat = true;
		CPUCombatSettingData.EnemySquadData = *CPUCombatData; // 이때만 복사
		CPUCombatSettingData.Players[0] = PS;
		
		SetupCombat(CPUCombatSettingData);
	}
	else
	{
		CombatPSMatchingQueue.Add(PS);
	}
}

void UNGCombatManagerComponent::StartCountingCombat()
{
	GetWorld()->GetTimerManager().SetTimer(FightStartTimerHandle, this, &UNGCombatManagerComponent::StartCombat, FightWaitTime, false);	
}

void UNGCombatManagerComponent::MatchingCombatUser()
{
	Algo::RandomShuffle(CombatPSMatchingQueue);
	
	uint8 Checker = 0;
	
	FCombatSettingData CombatSettingData;
	
	//TODO: 홀수일때 처리필요 (부전승)
	for (ANGPlayerState* PS : CombatPSMatchingQueue)
	{
		CombatSettingData.Players[Checker++] = PS;
		
		if (Checker > 1)
		{
			Checker = 0;
			SetupCombat(CombatSettingData);
			
			CombatSettingData.Reset();
		}
	}
}

void UNGCombatManagerComponent::RequestSpawnSquadByPlayer(ANGPlayerController* RequestingPC,
	const FEnemySquadData& SquadData)
{
	if (!GetOwner()->HasAuthority())	return;
	
	if (!RequestingPC) return;
	
	for (const FEnemySpawnInfo& EnemySpawnInfo : SquadData.SpawnUnits)
	{
		if (UNGSpawnHelper::SpawnEnemyPawn(RequestingPC, EnemySpawnInfo))
		{
		}
	}
}

void UNGCombatManagerComponent::ProcessPlayerFlee(ANGPlayerController* PlayerController)
{
	if (ANGPlayerState* PS = PlayerController->GetPlayerState<ANGPlayerState>())
	{
		// UE_LOG(LogTemp, Log, TEXT("ProcessPlayerFlee"));
		//TODO: 돈 구현하고 돈 넘겨주는거 만들기
		NotifyEndCombat(PS, ECombatResult::Lose);
	}
}

void UNGCombatManagerComponent::StartCombat()
{
	if (!GetOwner()->HasAuthority())	return;
	
	ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>();
	if (!GS)	return;
	
	UE_LOG(LogTemp, Log, TEXT("CombatDatas Num : %d"), CombatDatas.Num());
	
	//모든 유저 경기장 순회하면서 전투상태로 변경
	for (const FCombatSettingData& Data : CombatDatas)
	{
		for (ANGPlayerState* PS : Data.Players)
		{
			if (IsValid(PS))
			{
				PS->StartCombat();
			}
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

void UNGCombatManagerComponent::NotifyEndCombat(const ANGPlayerState* TargetPlayer, ECombatResult Result)
{
	//폰이 죽었을때 자기가 졌는지 확인하고 호출하는애라 CombatDatas에서 찾아서 반대편에 애는 승리처리
	if (!GetOwner()->HasAuthority())	return;

	UE_LOG(LogTemp, Log, TEXT("NotifyEndCombat"));
	
	int32 FoundDataIndex = INDEX_NONE;
	int32 TargetPlayerIndex = INDEX_NONE;
	
	for (int32 i = 0; i < CombatDatas.Num(); ++i)
	{
		if (CombatDatas[i].Players.IsValidIndex(0) && CombatDatas[i].Players[0] == TargetPlayer) { FoundDataIndex = i; TargetPlayerIndex = 0; break; }
		if (CombatDatas[i].Players.IsValidIndex(1) && CombatDatas[i].Players[1] == TargetPlayer) { FoundDataIndex = i; TargetPlayerIndex = 1; break; }
	}
	
	bool bIsNewNotification = false;
	
	if (FoundDataIndex != INDEX_NONE)
	{
		int32 WinnerPlayerIndex = Result == ECombatResult::Win ? TargetPlayerIndex : 1 - TargetPlayerIndex;
		const FCombatSettingData& TargetCombat = CombatDatas[FoundDataIndex];
		
		if (TargetCombat.Players.IsValidIndex(WinnerPlayerIndex))
		{
			if (ANGPlayerState* Winner = TargetCombat.Players[WinnerPlayerIndex].Get())
			{
				FCombatResultData ResultData;
				ResultData.EarnedGold = 100.f;
				
				if (CombatResultDictionary.Find(Winner) == nullptr)
				{
					ResultData.WinResult = ECombatResult::Win;
					CombatResultDictionary.Add(Winner, ResultData);
					bIsNewNotification = true;
				}else
				{
					uint8 WinCombinedResult = static_cast<uint8>(CombatResultDictionary[Winner].WinResult) | static_cast<uint8>(ECombatResult::Win);
					ResultData.WinResult = static_cast<ECombatResult>(WinCombinedResult);
					CombatResultDictionary[Winner] = ResultData;
				}
				
				Winner->FinishCombat();
			}
		}
		
		int32 LoserPlayerIndex = 1 - WinnerPlayerIndex;
		
		if (TargetCombat.Players.IsValidIndex(LoserPlayerIndex))
		{
			if (ANGPlayerState* Loser = TargetCombat.Players[LoserPlayerIndex].Get())
			{
				FCombatResultData ResultData;
				// 여기서 싸움 전 체력이랑 끝났을때 남은체력 비교해서 최소20%에서 최대 90%로 돈 뺏어가기
				ResultData.EarnedGold = -100.f;
				
				if (CombatResultDictionary.Find(Loser) == nullptr)
				{
					ResultData.WinResult = ECombatResult::Lose;
					CombatResultDictionary.Add(Loser, ResultData);
					bIsNewNotification = true;
				}else
				{
					uint8 LoseCombinedResult = static_cast<uint8>(CombatResultDictionary[Loser].WinResult) | static_cast<uint8>(ECombatResult::Lose);
					ResultData.WinResult = static_cast<ECombatResult>(LoseCombinedResult);
					CombatResultDictionary[Loser] = ResultData;
				}
				
				Loser->FinishCombat();
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
	
	constexpr static EGameState EndGameState = EGameState::Exploration;
	
	for (FCombatSettingData& Data : CombatDatas)
	{
		if (Data.Players.Num() > 0)
		{
			if (ANGPlayerState* HomePlayer = Data.Players[0].Get())
			{
				HomePlayer->SetGameState(EndGameState);
				HomePlayer->OnCombatEnd(CombatResultDictionary[HomePlayer]);
				
				ResetGrid(HomePlayer);
			}
		}
		
		if (Data.Players.Num() > 1)
		{
			if (ANGPlayerState* AwayPlayer = Data.Players[1].Get())
			{
				AwayPlayer->SetGameState(EndGameState);
				AwayPlayer->OnCombatEnd(CombatResultDictionary[AwayPlayer]);

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
	
	CombatPSMatchingQueue.Empty();
}

void UNGCombatManagerComponent::NotifyPawnDied(ANGPawnBase* DeadPawn)
{
	if (!GetOwner()->HasAuthority())	return;
	
	if (!DeadPawn)	return;
	
	ANGPlayerController* OwnerPC = DeadPawn->GetOwner<ANGPlayerController>();
	ANGPlayerState* OwnerPS = OwnerPC ? OwnerPC->GetPlayerState<ANGPlayerState>() : nullptr;
	
	//TODO: Enum으로 Type 넣어두던가 인터페이스 형태로? 바꾸는게 나을듯
	if (DeadPawn->IsA(ANGEnemyPawn::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("적 사망"));
		
		if (OwnerPS->IsCPUCombatFinished())
		{
			NotifyEndCombat(OwnerPS, ECombatResult::Win);
		}
		//적 사망 이벤트
		//적 사망 델리게이트 만들어서 구독시키게 하고 델리게이트 호출도 나쁘지 않을듯
	}
	else if (DeadPawn->IsA(ANGUnitPawn::StaticClass()))
	{
		UE_LOG(LogTemp, Log, TEXT("유닛 사망"));
		//유닛 죽었을때 이벤트
		if (UNGPocketComponent* Pocket = OwnerPS ? OwnerPS->GetPlayerPocket() : nullptr)
		{
			if (Pocket->IsAnnihilated())
			{
				NotifyEndCombat(OwnerPS, ECombatResult::Lose);
			}
		}
	}
}

void UNGCombatManagerComponent::SetupCombat(const FCombatSettingData& SettingData)
{
	if (!SettingData.Players[0])	return;
	if (!SettingData.Players[1] && !SettingData.bIsCPUCombat)	return;
	
	//전투 시작 전 그리드 상태 복구용 스냅샷
	for (ANGPlayerState* Player : SettingData.Players)
	{
		if (IsValid(Player))
		{
			Player->CaptureSnapShot();
			Player->SetGameState(EGameState::Combat);
		}
	}
	
	if (SettingData.bIsCPUCombat)
	{
		//CPU는 몹소환
		const FEnemySquadData& SquadData= SettingData.EnemySquadData;
		
		if (ANGPlayerState* Player = SettingData.Players[0].Get())
		{
			Player->InitCPUCombat(SquadData);
			
			if (ANGPlayerController* PC = Player ? Player->GetOwner<ANGPlayerController>() : nullptr)
			{
				RequestSpawnSquadByPlayer(PC, SquadData);
			}
		}

	}
	else
	{
		//유저는 카메라 이동 시켜줘야함
		if (AArenaManager* ArenaManager = SettingData.Players[1]->GetArenaManager())
		{
			FArenaAddress AwayArenaAddress(SettingData.Players[0]->GetHomeArena(), EPossessArenaIdentification::Away);
			ArenaManager->PossessArena(AwayArenaAddress);
		}
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