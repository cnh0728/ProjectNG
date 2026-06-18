#pragma once

#include "CoreMinimal.h"
#include "Core/NGEnemyDataAsset.h"
#include "NGCombatData.generated.h"

class ANGPlayerState;
class ANGEnemyPawn;

USTRUCT()
struct FCombatResultData
{
	GENERATED_BODY()
	bool bWin;
	//얻은 전리품같은거
};

USTRUCT()
struct FCombatSettingData
{
	GENERATED_BODY()
	
	FCombatSettingData()
	{
		Players.SetNum(2);
		bIsCPUCombat = false;
	}
	
	void Reset()
	{
		Players.Reset();
		Players.SetNum(2);
		bIsCPUCombat = false;
		EnemySquadData = FEnemySquadData();
	}
	
	UPROPERTY()
	bool bIsCPUCombat;
	
	UPROPERTY()
	TArray<TObjectPtr<ANGPlayerState>> Players;
	
	UPROPERTY()
	FEnemySquadData EnemySquadData;
};