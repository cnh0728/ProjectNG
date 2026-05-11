// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGGameState.h"

#include "Core/NGUnitData.h"
#include "Net/UnrealNetwork.h"

ANGGameState::ANGGameState() : GridMargin(3000.f)
{
}

void ANGGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ANGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGGameState, GridMapManagers);
}

uint8 ANGGameState::AddGridMapManager(AGridMapManager* InitGridMap)
{
	GridMapManagers.AddUnique(InitGridMap); 
	return GridMapManagers.Num() - 1;
}
