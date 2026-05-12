// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGGameState.h"

#include "Components/CombatManagerComponent.h"
#include "Net/UnrealNetwork.h"

ANGGameState::ANGGameState() : GridMargin(3000.f)
{
	CombatManagerComponent = CreateDefaultSubobject<UCombatManagerComponent>(TEXT("CombatManager"));
}

void ANGGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ANGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGGameState, PlayerStates);
}

uint8 ANGGameState::RegisterPlayer(ANGPlayerState* InPS)
{
	PlayerStates.Add(InPS);
	return PlayerStates.Num() - 1;
}