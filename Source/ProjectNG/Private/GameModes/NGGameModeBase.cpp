// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "GameModes/NGGameModeBase.h"

#include "Game/NGGameState.h"
#include "Player/NGPlayerState.h"

void ANGGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (ANGPlayerState* PS = NewPlayer->GetPlayerState<ANGPlayerState>())
	{
		if (ANGGameState* GS = GetGameState<ANGGameState>())
		{
			uint32 AssignedIndex = GS->PlayerArray.Num() - 1;
			PS->InitializeLogin(AssignedIndex);
		}
	}
}
