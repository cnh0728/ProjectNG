// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGGameplayTags.h"

#include "GameplayTagsManager.h"

FNGGameplayTags FNGGameplayTags::GameplayTags;

void FNGGameplayTags::InitializeNativeTags()
{
	GameplayTags.Node_Town_Elf = UGameplayTagsManager::Get().AddNativeGameplayTag(
	FName("Node.Town.Elf"),FString("Elf Town Node"));
	
	GameplayTags.Node_Town_Dwarf = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Town.Dwarf"), FString("Dwarf Town Node"));
	
	GameplayTags.Node_Event_Default = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Event.Default"), FString("Default Event Node"));
	
	GameplayTags.Node_Event_Combat = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Event.Combat"), FString("Combat Event Node"));
	
	GameplayTags.Node_Event_Hidden = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Event.Hidden"), FString("Hidden Event Node"));
}
