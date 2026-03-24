// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGGameModeBase.h"
#include "NGCombatGameMode.generated.h"

class ANGCharacterBase;
/**
 * 
 */
UCLASS()
class PROJECTNG_API ANGCombatGameMode : public ANGGameModeBase
{
	GENERATED_BODY()
	
public:
	void CharacterDied(ANGCharacterBase* DeadEnemy);
	
	void InitCombatSetting(int32 EnemyCount);
	
protected:
	int32 CurrentEnemyCount = 0;
	int32 TargetKillCount = 10;
};
