// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGGameplayAbility_Attack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/NGUnitCharacter.h"

void UNGGameplayAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ANGUnitCharacter* Unit = GetUnitCharacterFromActorInfo();
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("UNGGameplayAbility_Attack::ActivateAbility - No Unit"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("ActivateAbility"));
	
	UAnimMontage* MontageToPlay = Unit->GetAttackMontage();
	
	float CurrentAS = GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UNGAttributeSet::GetAttackSpeedAttribute());
	
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* AttackTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("AttackTask"), MontageToPlay, CurrentAS);
		AttackTask->OnCompleted.AddDynamic(this, &UNGGameplayAbility_Attack::K2_EndAbility);
		AttackTask->OnInterrupted.AddDynamic(this, &UNGGameplayAbility_Attack::K2_EndAbility);
		AttackTask->OnCancelled.AddDynamic(this, &UNGGameplayAbility_Attack::K2_EndAbility);
		
		AttackTask->ReadyForActivation();
	}else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
	
}
