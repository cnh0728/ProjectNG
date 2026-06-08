// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGUnitDataManager.h"

void UNGUnitDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const FString AbilityDataTablePath = TEXT("/Game/DataTables/DT_UnitAbilityData.DT_UnitAbilityData"); 
	UnitAbilityDataTable = LoadObject<UDataTable>(nullptr, *AbilityDataTablePath);
	
	const FString UnitDataTablePath = TEXT("/Game/DataTables/DT_UnitData_kr.DT_UnitData_kr"); 
	UnitDataTable = LoadObject<UDataTable>(nullptr, *UnitDataTablePath);
	
	if (UnitAbilityDataTable)
	{
		TArray<FUnitAbilityData*> AllRows;
		UnitAbilityDataTable->GetAllRows<FUnitAbilityData>(TEXT("InitializeUnitMap"), AllRows);

		for (FUnitAbilityData* RowData : AllRows)
		{
			if (RowData && RowData->IdentificationTag.IsValid())
			{
				TagToUnitAbilityDataMap.Add(RowData->IdentificationTag, RowData);
			}
		}
        
		UE_LOG(LogTemp, Log, TEXT("유닛 능력치 데이터 테이블 캐싱 완료! 총 %d개"), TagToUnitAbilityDataMap.Num());
	}
	
	if (UnitDataTable)
	{
		for (auto& Pair : UnitDataTable->GetRowMap())
		{
			FName RowName = Pair.Key;
			FUnitData* RowData = (FUnitData*)Pair.Value;
		
			if (RowData && RowData->IdentificationTag.IsValid())
			{
				TagToUnitDataMap.Add(RowData->IdentificationTag, RowData);
				TagToUnitNameDataMap.Add(RowData->IdentificationTag, RowName);
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("유닛 데이터 테이블 캐싱 완료! 총 %d개"), TagToUnitDataMap.Num());
	}
}

const FUnitAbilityData* UNGUnitDataManager::GetUnitAbilityData(const FGameplayTag IdentificationTag)
{
	if (!IdentificationTag.IsValid()) return nullptr;
	
	if (FUnitAbilityData* const* FoundData = TagToUnitAbilityDataMap.Find(IdentificationTag))
	{
		return *FoundData; // 포인터의 포인터이므로 한 번 역참조해서 반환
	}
	
	UE_LOG(LogTemp, Warning, TEXT("해당 태그(%s)를 가진 유닛 데이터가 없습니다!"), *IdentificationTag.ToString());
	return nullptr;
}

const FUnitData* UNGUnitDataManager::GetUnitData(const FGameplayTag IdentificationTag)
{
	if (!IdentificationTag.IsValid()) return nullptr;
	
	if (FUnitData* const* FoundData = TagToUnitDataMap.Find(IdentificationTag))
	{
		return *FoundData; // 포인터의 포인터이므로 한 번 역참조해서 반환
	}
	
	UE_LOG(LogTemp, Warning, TEXT("해당 태그(%s)를 가진 유닛 데이터가 없습니다!"), *IdentificationTag.ToString());
	return nullptr;
}

FName UNGUnitDataManager::GetUnitName(const FGameplayTag IdentificationTag) const
{
	if (const FName* FoundRowName = TagToUnitNameDataMap.Find(IdentificationTag))
	{
		return *FoundRowName;
	}
	return NAME_None;
}
