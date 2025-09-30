// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/Game/NGGameState.h"

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

bool ANGGameState::GrabUnitFromPool(const TSubclassOf<AActor> UnitClass)
{
	if (!HasAuthority()) return false;

	if (int32* Count = UnitPool.Find(UnitClass))
	{
		if (*Count > 0)
		{
			(*Count)--;
			return true;
		}
	}
	return false;
}

void ANGGameState::ReturnUnitToPool(const TSubclassOf<AActor> UnitClass)
{
	if (!HasAuthority()) return;

	if (UnitPool.Contains(UnitClass))
	{
		UnitPool[UnitClass]++;
	}
}

TSubclassOf<AActor> ANGGameState::GetRandomUnitByTier(EUnitTier Tier)
{
	if (!HasAuthority()) return nullptr;

	if (TieredUnitPool.Contains(Tier))
	{
		TArray<TSubclassOf<AActor>> AvailableUnits;
		const TArray<TSubclassOf<AActor>>& UnitsInTier = TieredUnitPool[Tier];

		// 현재 남아 있는 유닛만 필터링
		for (TSubclassOf UnitClass : UnitsInTier)
		{
			if (UnitPool.Contains(UnitClass) && UnitPool[UnitClass] > 0)
			{
				AvailableUnits.Add(UnitClass);
			}
		}

		if (AvailableUnits.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, AvailableUnits.Num() - 1);
			return AvailableUnits[RandomIndex];
		}
	}
	return nullptr;
}

void ANGGameState::InitializeUnitPool()
{
	if (!UnitDataTable.Get()->IsValidLowLevel()) return;

	FString ContextString;
	TArray<FName> RowNames = UnitDataTable.Get()->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FUnitData* Row = UnitDataTable.Get()->FindRow<FUnitData>(RowName, ContextString);
		if (Row && Row->UnitClass)
		{
			// 총 개수 추가
			UnitPool.Add(Row->UnitClass, Row->TotalCountInPool);

			// 등급별로 유닛 클래스 추가
			TieredUnitPool.FindOrAdd(Row->Tier).Add(Row->UnitClass);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Unit Pool Initialized on Server."));
}
