// Copyright (c) 2025 TeamNG. All Rights Reserved.

#define DEFAULT_REPLICATION_IMPLEMENTATION(ClassName, Name) \
	void ClassName::OnRep_##Name(const FGameplayAttributeData& OldValue) const \
	{ GAMEPLAYATTRIBUTE_REPNOTIFY(ClassName, Name, OldValue); }

#include "AbilitySystem/NGAttributeSet.h"

#include "Net/UnrealNetwork.h"

void UNGAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Income, COND_None, REPNOTIFY_Always);
}

DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Health)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, MaxHealth)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Mana)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, MaxMana)

DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, AttackDamage)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, AttackSpeed)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Defense)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Income)