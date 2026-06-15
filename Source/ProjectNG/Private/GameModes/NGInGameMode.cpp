// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "GameModes/NGInGameMode.h"

#include "Components/NGCombatManagerComponent.h"
#include "Core/NGDeveloperSettings.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerState.h"


ANGInGameMode::ANGInGameMode()
{
	CombatManagerComponent = CreateDefaultSubobject<UNGCombatManagerComponent>(TEXT("CombatManager"));
}

void ANGInGameMode::RequestStartCombat(APlayerController* PC, bool bIsCPUCombat)
{
	// TODO: 테스트용으로 잠시 막음
	// if (CurrentState == EGameState::Combat)	return;
	
	ANGGameState* GS = GetGameState<ANGGameState>();
		
	// 테스트용
	if (GS)
	{
		for (APlayerState* RawPS : GS->PlayerArray)
		{
			if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
			{
				if (bIsCPUCombat)
				{
					TSoftObjectPtr<UNGEnemyDataAsset> SoftPath = GetDefault<UNGDeveloperSettings>()->EnemyDataAsset;

					if (SoftPath.IsNull()) return;

					UNGEnemyDataAsset* LoadedAsset = SoftPath.LoadSynchronous();
	    
					if (LoadedAsset)
					{
						FEnemySquadData SelectedData;
						if (LoadedAsset->GetRandomSquadForZone(PS->GetCurrentZoneTag(), SelectedData))
						{
							CombatManagerComponent->EnqueueCombatPhase(PS, &SelectedData);
						}
					}
				}
				else
				{
					CombatManagerComponent->EnqueueCombatPhase(PS);
				}
			}
		}
		
		CombatManagerComponent->MatchingCombatUser();
		CombatManagerComponent->StartCountingCombat();
	}
}

void ANGInGameMode::OnGameStart()
{
	ANGGameState* GS = GetGameState<ANGGameState>();
	NotifyGameStartToPlayer(GS);
}

void ANGInGameMode::NotifyGameStartToPlayer(ANGGameState* GS)
{
	if (!GS)	return;
	
	for (APlayerState* RawPS : GS->PlayerArray)
	{
		if (ANGPlayerState* PS = Cast<ANGPlayerState>(RawPS))
		{
			PS->SetGameState(EGameState::Exploration);
		}
	}	
}

void ANGInGameMode::OnCombatFinished(const FCombatResultData& ResultData)
{
	UE_LOG(LogTemp, Log, TEXT("Combat Finished"));
		
	//ResultData로 점수나 그런거 반영하기
	
}

void ANGInGameMode::ReportPawnDeath(ANGPawnBase* DeadPawn) const
{
	if (CombatManagerComponent)
	{
		CombatManagerComponent->NotifyPawnDied(DeadPawn);
	}
	
}

void ANGInGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Only server
	InitializeUnitPool();
}



int32 ANGInGameMode::GrabUnitFromPool(FName UnitRowName)
{
	if (!HasAuthority()) return false;
	
	if (int32* Count = UnitPool.Find(UnitRowName))
	{
		if (*Count > 0)
		{
			(*Count)--;
			return *Count;
		}
	}
	return -1;
}

bool ANGInGameMode::IsExistUnit(FName UnitRowName)
{
	return *UnitPool.Find(UnitRowName) > 0;
}

bool ANGInGameMode::IsExistUnitDataTable()
{
	return UnitDataTable != nullptr;
}

void ANGInGameMode::ReturnUnitToPool(FName UnitRowName, int32 UnitCount)
{
	if (int32* Count = UnitPool.Find(UnitRowName))
	{
		(*Count) += UnitCount;
	}
}

FName ANGInGameMode::GetRandomUnitByTier(EUnitTier Tier)
{
	if (TieredUnitPool.Contains(Tier))
	{
		TArray<FName> AvailableUnits;
		const TArray<FName>& UnitsInTier = TieredUnitPool[Tier];

		// 현재 남아 있는 유닛만 필터링
		for (FName UnitRowName : UnitsInTier)
		{
			if (UnitPool.Contains(UnitRowName) && UnitPool[UnitRowName] > 0)
			{
				AvailableUnits.Add(UnitRowName);
			}
		}

		if (AvailableUnits.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, AvailableUnits.Num() - 1);
			return AvailableUnits[RandomIndex];
		}
	}
	return NAME_None;
}

TSubclassOf<ANGUnitPawn> ANGInGameMode::GetUnitClass(FName UnitName) const
{
	FUnitData* FoundRow = UnitDataTable->FindRow<FUnitData>(UnitName, TEXT(""));
	if (!FoundRow)	return nullptr;
	
	return FoundRow->UnitClass;
}

void ANGInGameMode::InitializeUnitPool()
{
	if (!IsValid(UnitDataTable.Get())) return;

	TArray<FName> RowNames = UnitDataTable.Get()->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FString ContextString;
		FUnitData* Row = UnitDataTable.Get()->FindRow<FUnitData>(RowName, ContextString);
		if (Row && Row->UnitClass)
		{
			// 총 개수 추가
			UnitPool.Add(RowName, Row->TotalCountInPool);

			// 등급별로 유닛 클래스 추가
			TieredUnitPool.FindOrAdd(Row->Tier).Add(RowName);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Unit Pool Initialized on Server."));
}
