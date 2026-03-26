// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGGameModeBase.h"
#include "Combat/NGCombatData.h"
#include "NGInGameGameMode.generated.h"

/**
 * 
 */

class AGridMapManager;
class ANGCharacterBase;
class ACombatManager;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Waiting,
	Exploration,
	Combat,
	GameOver
};

UCLASS()
class PROJECTNG_API ANGInGameGameMode : public ANGGameModeBase
{
	GENERATED_BODY()
	
	
public:
	void RequestStartCombat();
	void OnCombatFinished(const FCombatResultData& ResultData);
	void ReportCharacterDeath(ANGCharacterBase* DeadCharacter);

	void InitializeGridMapManager(AGridMapManager* InitGridMap){ GridMapManager = InitGridMap; }
	
	AGridMapManager* GetGridMapManager(){ return GridMapManager; }
	
	ACombatManager* GetCombatManager(){ return ActiveCombatManager; }
	
protected:
	void ChangeState(EGameState NewState);	

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameState CurrentState;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ACombatManager> CombatManagerClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Managers")
	TObjectPtr<ACombatManager> ActiveCombatManager;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Managers")
	TObjectPtr<AGridMapManager> GridMapManager;
};
