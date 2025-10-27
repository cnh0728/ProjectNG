// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NGUnitData.h"
#include "GameFramework/GameStateBase.h"
#include "NGGameState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API ANGGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ANGGameState();
	
	virtual void BeginPlay() override;

	// if true, grab the unit from pool.
	bool GrabUnitFromPool(TSubclassOf<AActor> UnitClass);

	// return the unit to pool
	void ReturnUnitToPool(TSubclassOf<AActor> UnitClass);

	// return unit of specified tier, to random.
	TSubclassOf<AActor> GetRandomUnitByTier(EUnitTier Tier);

protected:
	// Key: Unit class, Value: remain count
	TMap<TSubclassOf<AActor>, int32> UnitPool;

	// Tier unit pool
	TMap<EUnitTier, TArray<TSubclassOf<AActor>>> TieredUnitPool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|DataTable")
	TObjectPtr<UDataTable> UnitDataTable;

private:
	void InitializeUnitPool();
};
