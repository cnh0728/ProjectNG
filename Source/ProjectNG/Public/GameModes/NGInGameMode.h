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
	
	// Game Flow
	void StartTurn();
	void StartNodeSelection();
	void OnNodeSelectionTimerTick();
	void ProcessNodeSelection(AController* Controller, int32 NodeID);
	void CheckAllPlayersReadyForNodeSelection();
	
	void StartActionPhase();
	void OnActionPhaseTimerTick();
	void CheckAllPlayersFinishedAction();
	
	void EndTurn();
	
	virtual void BeginPlay() override;

public:
	float GetUnitPrice(ANGUnitPawn* Unit) const;
	
	//유닛을 팔고(반환까지 진행) 유닛의 값을 리턴
	int32 SellUnit(ANGUnitPawn* Unit);
	
	/** 유닛풀에서 유닛 카운트를 하나 감소시킵니다. 이후 감소된 카운트를 반환합니다.
 *  반환할 수 없다면 -1을 반환합니다.
 */
	int32 GrabUnitFromPool(FGameplayTag UnitTag);

	/** true일 경우, UnitPool에 유닛이 1개 이상 존재합니다. */
	bool IsExistUnit(FGameplayTag UnitTag);

	bool IsExistUnitDataTable() const;
	
	/** UnitPool로 유닛을 UnitCount만큼 반환합니다. */
	void ReturnUnitToPool(FGameplayTag UnitTag, int32 UnitCount = 1);

	// 특정 티어의 유닛을 랜덤으로 반환합니다.
	FGameplayTag GetRandomUnitByTier(EUnitTier Tier);


	bool CanBuyUnit(FGameplayTag UnitTag, float OwnedGold) const;

	TSubclassOf<ANGUnitPawn> GetUnitClass(FGameplayTag UnitTag) const;
	const FUnitData* GetUnitData(FGameplayTag UnitTag) const;

	UNGCombatManagerComponent* GetCombatManagerComponent() { return CombatManagerComponent; };	
protected:
	void InitializeUnitPool();

	// Key: GameplayTag, Value: remain count
	TMap<FGameplayTag, int32> UnitPool;

	// Tier unit pool
	TMap<EUnitTier, TArray<FGameplayTag>> TieredUnitPool;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	TObjectPtr<UNGCombatManagerComponent> CombatManagerComponent;

	FTimerHandle PhaseTimerHandle;
};
