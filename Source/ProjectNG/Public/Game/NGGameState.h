// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NGGameState.generated.h"

class AGridMapManager;
enum class EUnitTier : uint8;
class ACombatManager;
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

	/** 유닛풀에서 유닛 카운트를 하나 감소시킵니다. 이후 감소된 카운트를 반환합니다.
	 *  반환할 수 없다면 -1을 반환합니다.
	 */
	int32 GrabUnitFromPool(FName UnitRowName);

	/** true일 경우, UnitPool에 유닛이 1개 이상 존재합니다. */
	bool IsExistUnit(FName UnitRowName);

	/** UnitPool로 유닛을 UnitCount만큼 반환합니다. */
	void ReturnUnitToPool(FName UnitRowName, int32 UnitCount = 1);

	// 특정 티어의 유닛을 랜덤으로 반환합니다.
	FName GetRandomUnitByTier(EUnitTier Tier);

	UDataTable* GetUnitDataTable(){ return UnitDataTable; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
	uint8 InitializeGridMapManager(AGridMapManager* InitGridMap);
	
	AGridMapManager* GetGridMapManager(uint8 Id){ return GridMapManagers[Id]; }
	
protected:
	// Key: DataTable RowName, Value: remain count
	TMap<FName, int32> UnitPool;

	// Tier unit pool
	TMap<EUnitTier, TArray<FName>> TieredUnitPool;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|DataTable")
	TObjectPtr<UDataTable> UnitDataTable;
	
	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Managers")
	TArray<AGridMapManager*> GridMapManagers;
	
private:
	void InitializeUnitPool();
};
