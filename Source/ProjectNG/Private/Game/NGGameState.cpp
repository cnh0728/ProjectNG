// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGGameState.h"

ANGGameState::ANGGameState()
{
}

void ANGGameState::BeginPlay()
{
	Super::BeginPlay();

	// Only server
	if (HasAuthority())
	{
		InitializeUnitPool();
	}
}

bool ANGGameState::GrabUnitFromPool(FName UnitRowName)
{
	if (!HasAuthority()) return false;

	if (int32* Count = UnitPool.Find(UnitRowName))
	{
		if (*Count > 0)
		{
			(*Count)--;
			return true;
		}
	}
	return false;
}

void ANGGameState::ReturnUnitToPool(FName UnitRowName)
{
	if (!HasAuthority()) return;

	if (int32* Count = UnitPool.Find(UnitRowName))
	{
		(*Count)++;
	}
}

FName ANGGameState::GetRandomUnitByTier(EUnitTier Tier)
{
	if (!HasAuthority()) return NAME_None;

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

void ANGGameState::InitializeUnitPool()
{
	if (!UnitDataTable.Get()->IsValidLowLevel()) return;

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
