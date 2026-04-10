// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NGUnitData.h"
#include "Core/UnitAbilityDataRow.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NGUnitDataManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API UNGUnitDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	const FUnitAbilityData* GetUnitAbilityData(FGameplayTag IdentificationTag);
	
	const FUnitData* GetUnitData(FGameplayTag IdentificationTag);
	
	const UDataTable* GetUnitDataTable() { return UnitDataTable; };
	
	const UDataTable* GetUnitAbilityDataTable() { return UnitAbilityDataTable; };
	
private:
	UPROPERTY()
	TObjectPtr<UDataTable> UnitAbilityDataTable;
	
	UPROPERTY()
	TObjectPtr<UDataTable> UnitDataTable;
	
	TMap<FGameplayTag, FUnitAbilityData*> TagToUnitAbilityDataMap;
	
	TMap<FGameplayTag, FUnitData*> TagToUnitDataMap;
};
