// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGEnemyDataAsset.h"

bool UNGEnemyDataAsset::GetRandomSquadForZone(const FGameplayTag& TargetZone, FEnemySquadData& OutSquadData) const
{
	if (ZoneSquadMasterPool.Find(TargetZone))
	{
		const FZoneSquadPool& ZonePool = ZoneSquadMasterPool[TargetZone];
		
		if (ZonePool.SquadCandidates.IsEmpty())	return false;
		
		int32 RandomIndex = FMath::RandRange(0, ZonePool.SquadCandidates.Num() - 1);
		OutSquadData = ZonePool.SquadCandidates[RandomIndex];
		return true;
	}

	return false;
}
