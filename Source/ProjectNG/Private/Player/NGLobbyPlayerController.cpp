// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Player/NGLobbyPlayerController.h"
#include "Game/NGGameInstance.h"
#include "Player/NGLobbyPlayerState.h"

ANGLobbyPlayerController::ANGLobbyPlayerController()
{
	
}

void ANGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalPlayerController())
	{
		if (const UNGGameInstance* GI = Cast<UNGGameInstance>(GetWorld()->GetGameInstance()))
		{
			SetUserData(GI->PlayerName, true);
		}
	}
}

void ANGLobbyPlayerController::SetUserData_Implementation(const FString& PlayerName, bool bReady)
{
	ANGLobbyPlayerState* PS = GetPlayerState<ANGLobbyPlayerState>();
	if (PS)
	{
		PS->SetPlayerName(PlayerName);
	}
}
