// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGGameInstance.h"

#include "Core/NGGameplayTags.h"

void UNGGameInstance::Init()
{
	Super::Init();
	
	FNGGameplayTags::InitializeNativeTags();
	
	UE_LOG(LogTemp, Log, TEXT("GameInstance Init: Gameplay Tags Initialized"));
}
