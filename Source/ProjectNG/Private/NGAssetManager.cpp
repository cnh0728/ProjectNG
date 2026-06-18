// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "NGAssetManager.h"

#include "AbilitySystemGlobals.h"
#include "NGGameplayTags.h"

UNGAssetManager& UNGAssetManager::Get()
{
	check(GEngine);
	return *Cast<UNGAssetManager>(GEngine->AssetManager);
}

void UNGAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	FNGGameplayTags::InitializeNativeGameplayTags();
	
	UAbilitySystemGlobals::Get().InitGlobalData();
}
