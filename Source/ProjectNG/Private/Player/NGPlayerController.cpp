// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Player/NGPlayerController.h"

#include "Components/NGPocketComponent.h"

ANGPlayerController::ANGPlayerController()
{
	PlayerPocket = CreateDefaultSubobject<UNGPocketComponent>("PocketComponent");
}

void ANGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);
}
