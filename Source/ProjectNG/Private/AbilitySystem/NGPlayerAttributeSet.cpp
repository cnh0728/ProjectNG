// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGPlayerAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UNGPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UNGPlayerAttributeSet, Gold, COND_None, REPNOTIFY_Always);
}

DEFAULT_REPLICATION_IMPLEMENTATION(UNGPlayerAttributeSet, Gold)

void UNGPlayerAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}
