// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/JobSkill/NGGameplayAbility_JobSkill.h"

void UNGGameplayAbility_JobSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                  const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                  const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	
	TossTargetData = *TriggerEventData;
}
