// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "GameModes/NGCombatGameMode.h"

void ANGCombatGameMode::CharacterDied(ANGCharacterBase* DeadEnemy)
{
}

void ANGCombatGameMode::InitCombatSetting(int32 EnemyCount)
{
	CurrentEnemyCount = 0;
	TargetKillCount = EnemyCount;
}
