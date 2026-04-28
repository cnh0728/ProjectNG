// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Player/NGPlayerState.h"

#include "Components/NGPocketComponent.h"
#include "Net/UnrealNetwork.h"

ANGPlayerState::ANGPlayerState()
{
	// GAS를 원활히 사용하기 위함
	SetNetUpdateFrequency(100.0f);

	AbilitySystemComponent = CreateDefaultSubobject<UNGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	PlayerPocket = CreateDefaultSubobject<UNGPocketComponent>("PocketComponent");
}

void ANGPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGPlayerState, GridMap);
	DOREPLIFETIME(ANGPlayerState, PlayerPocket);
}
