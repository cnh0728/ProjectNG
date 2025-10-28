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

	// true일 경우, UnitPool에서 유닛 하나를 가져옵니다.
	bool GrabUnitFromPool(TSubclassOf<AActor> UnitClass);

	// UnitPool로 유닛을 반환합니다.
	void ReturnUnitToPool(TSubclassOf<AActor> UnitClass);

	// 특정 티어의 유닛을 랜덤으로 반환합니다.
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
