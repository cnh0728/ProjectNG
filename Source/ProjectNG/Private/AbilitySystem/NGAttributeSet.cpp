// Copyright (c) 2025 TeamNG. All Rights Reserved.
#include "AbilitySystem/NGAttributeSet.h"

#include "Character/NGCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

#define DEFAULT_REPLICATION_IMPLEMENTATION(ClassName, Name) \
	void ClassName::OnRep_##Name(const FGameplayAttributeData& OldValue) \
	{ GAMEPLAYATTRIBUTE_REPNOTIFY(ClassName, Name, OldValue); }

void UNGAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, AttackDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, AbilityPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, AttackRange, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, CriticalRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, DodgeRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, PhysicalDefense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, MagicDefense, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Agility, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, TargetCount, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UNGAttributeSet, Income, COND_None, REPNOTIFY_Always);
}

DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Health)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, MaxHealth)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Mana)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, MaxMana)

DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, AttackDamage)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, AbilityPower)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, AttackRange)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, AttackSpeed)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, CriticalRate)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, DodgeRate)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, PhysicalDefense)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, MagicDefense)

DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Strength)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Intelligence)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Agility)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Defense)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, TargetCount)
DEFAULT_REPLICATION_IMPLEMENTATION(UNGAttributeSet, Income)

void UNGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute() && GetHealth() <= 0.f)
	{
		if (ANGCharacterBase* TargetCharacter = Cast<ANGCharacterBase>(Data.Target.GetAvatarActor()))
		{
			TargetCharacter->Die();
		}
	}
}
