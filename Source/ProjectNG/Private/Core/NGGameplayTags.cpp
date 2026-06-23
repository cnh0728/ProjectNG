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

	// 노드 타입 태그 추가
	GameplayTags.Node_Empty = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Empty"), FString("Empty Node"));
	GameplayTags.Node_Shop = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Shop"), FString("Shop Node"));
	GameplayTags.Node_Rest = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Rest"), FString("Rest Node"));
	GameplayTags.Node_Elite = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Node.Elite"), FString("Elite Node"));

	// 마을 버프 태그 추가
	GameplayTags.TownBuff_ExtraGold = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("TownBuff.ExtraGold"), FString("Extra Gold Buff"));
	GameplayTags.TownBuff_AttackBoost = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("TownBuff.AttackBoost"), FString("Attack Boost Buff"));
	GameplayTags.TownBuff_DefenseBoost = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("TownBuff.DefenseBoost"), FString("Defense Boost Buff"));
	GameplayTags.TownBuff_StartUnit = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("TownBuff.StartUnit"), FString("Start Unit Buff"));
}
