// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "GameModes/NGInGameGameMode.h"

#include "Combat/CombatManager.h"
#include "Core/NGUnitData.h"


void ANGInGameGameMode::RequestStartCombat(APlayerController* PC)
{
	if (CurrentState == EGameState::Combat)	return;
	
	if (!ActiveCombatManager)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ActiveCombatManager = GetWorld()->SpawnActor<ACombatManager>(CombatManagerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
	
	//ActiveCombatManager가 없을수 없지만 혹시모르니
	if (ActiveCombatManager)
	{
		CurrentState = EGameState::Combat;
		
		//이거는 따로 웨이브 관리하는거 만들어서 그때 값 가져와서 넣어야할듯?
		FCombatSettingData SettingData;
		SettingData.EnemyCount = 3;
		
		ActiveCombatManager->StartCombat(SettingData, PC);
	}
}

void ANGInGameGameMode::OnCombatFinished(const FCombatResultData& ResultData)
{
	if (CurrentState != EGameState::Combat)	return;
	
	UE_LOG(LogTemp, Log, TEXT("Combat Finished"));
	
	//Combat은 모험중이 아니면 들어갈 수 없음
	CurrentState = EGameState::Exploration;
	
	//ResultData로 점수나 그런거 반영하기

}

void ANGInGameGameMode::ReportPawnDeath(ANGPawnBase* DeadPawn)
{
	if (ActiveCombatManager)
	{
		ActiveCombatManager->PawnDied(DeadPawn);
	}
}

void ANGInGameGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Only server
	InitializeUnitPool();
}

void ANGInGameGameMode::ChangeState(const EGameState NewState)
{
	CurrentState = NewState;
}


int32 ANGInGameGameMode::GrabUnitFromPool(FName UnitRowName)
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

bool ANGInGameGameMode::IsExistUnit(FName UnitRowName)
{
	return *UnitPool.Find(UnitRowName) > 0;
}

bool ANGInGameGameMode::IsExistUnitDataTable()
{
	return UnitDataTable != nullptr;
}

void ANGInGameGameMode::ReturnUnitToPool(FName UnitRowName, int32 UnitCount)
{
	if (int32* Count = UnitPool.Find(UnitRowName))
	{
		(*Count) += UnitCount;
	}
}

FName ANGInGameGameMode::GetRandomUnitByTier(EUnitTier Tier)
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

TSubclassOf<ANGUnitPawn> ANGInGameGameMode::GetUnitClass(FName UnitName) const
{
	FUnitData* FoundRow = UnitDataTable->FindRow<FUnitData>(UnitName, TEXT(""));
	if (!FoundRow)	return nullptr;
	
	return FoundRow->UnitClass;
}

void ANGInGameGameMode::InitializeUnitPool()
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
