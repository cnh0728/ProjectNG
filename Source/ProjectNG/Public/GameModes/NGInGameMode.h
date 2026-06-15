// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGGameModeBase.h"
#include "Combat/NGCombatData.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "NGInGameMode.generated.h"

/**
 * 
 */

class ANGPawnBase;

UCLASS()
class PROJECTNG_API ANGInGameMode : public ANGGameModeBase
{
	GENERATED_BODY()

	ANGInGameMode();
	
public:
	void RequestStartCombat(APlayerController* PC, bool bIsCPUCombat);
	void OnGameStart();
	void NotifyGameStartToPlayer(ANGGameState* GS);
	void OnCombatFinished(const FCombatResultData& ResultData);
	void ReportPawnDeath(ANGPawnBase* DeadPawn) const;
	
	virtual void BeginPlay() override;

public:
	/** 유닛풀에서 유닛 카운트를 하나 감소시킵니다. 이후 감소된 카운트를 반환합니다.
 *  반환할 수 없다면 -1을 반환합니다.
 */
	int32 GrabUnitFromPool(FName UnitRowName);

	/** true일 경우, UnitPool에 유닛이 1개 이상 존재합니다. */
	bool IsExistUnit(FName UnitRowName);

	bool IsExistUnitDataTable();
	
	/** UnitPool로 유닛을 UnitCount만큼 반환합니다. */
	void ReturnUnitToPool(FName UnitRowName, int32 UnitCount = 1);

	// 특정 티어의 유닛을 랜덤으로 반환합니다.
	FName GetRandomUnitByTier(EUnitTier Tier);

	TSubclassOf<ANGUnitPawn> GetUnitClass(FName UnitName) const;

	UNGCombatManagerComponent* GetCombatManagerComponent() { return CombatManagerComponent; };	
protected:
	void InitializeUnitPool();

	// Key: DataTable RowName, Value: remain count
	TMap<FName, int32> UnitPool;

	// Tier unit pool
	TMap<EUnitTier, TArray<FName>> TieredUnitPool;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|DataTable")
	TObjectPtr<UDataTable> UnitDataTable;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	TObjectPtr<UNGCombatManagerComponent> CombatManagerComponent;
};
