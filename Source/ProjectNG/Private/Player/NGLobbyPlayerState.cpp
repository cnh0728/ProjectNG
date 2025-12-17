// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Player/NGLobbyPlayerState.h"

#include "Net/UnrealNetwork.h"

ANGLobbyPlayerState::ANGLobbyPlayerState()
{
	SetNetUpdateFrequency(100.f);
}

void ANGLobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void ANGLobbyPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGLobbyPlayerState, bReady);
}
