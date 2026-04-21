#pragma once

#include "CoreMinimal.h"
#include "NGCombatData.generated.h"

class ANGEnemyPawn;

USTRUCT(BlueprintType)
struct FWaveData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ANGEnemyPawn> EnemyClass;
	
	UPROPERTY(EditAnywhere)
	int32 EnemyCount = 10;
	
	UPROPERTY(EditAnywhere)
	float SpawnInterval = 1.0f; //스폰 간격
	
	UPROPERTY(EditAnywhere)
	float SpawnRandomDevation = 1.0f; //간격에서 랜덤 간극
};

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
	int EnemyCount;
};