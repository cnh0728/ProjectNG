// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGGameState.h"

#include "Components/NGCombatManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerState.h"

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
	
}

uint8 ANGGameState::GetPlayerId(const ANGPlayerState* InPS)
{
	return InPS->GetPlayerId();
}