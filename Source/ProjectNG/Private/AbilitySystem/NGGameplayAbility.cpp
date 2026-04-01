// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Character/NGUnitCharacter.h"

UNGGameplayAbility::UNGGameplayAbility()
{
	//인스턴싱 정책 (유닛마다 독립적인 쿨다운/데이터 관리를 위해 필수
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	ActivationBlockedTags.AddTag(DeadTag);
}

void UNGGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ANGCharacterBase* Unit = GetUnitCharacterFromActorInfo();
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("UNGGameplayAbility::ActivateAbility - No Unit"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ANGCharacterBase* Target = Unit->GetCurrentTarget();
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Target is null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAnimMontage* MontageToPlay = Unit->GetAttackMontage();
	
	float CurrentAS = GetAbilitySystemComponentFromActorInfo()->GetNumericAttribute(UNGAttributeSet::GetAttackSpeedAttribute());
	
	if (MontageToPlay)
	{
		UAbilityTask_PlayMontageAndWait* AbilityTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("Anim"), MontageToPlay, CurrentAS);
		AbilityTask->OnCompleted.AddDynamic(this, &UNGGameplayAbility::K2_EndAbility);
		AbilityTask->OnInterrupted.AddDynamic(this, &UNGGameplayAbility::K2_EndAbility);
		AbilityTask->OnCancelled.AddDynamic(this, &UNGGameplayAbility::K2_EndAbility);
		AbilityTask->ReadyForActivation();
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("UNGGameplayAbility::ActivateAbility - No MontageToPlay"));
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

ANGCharacterBase* UNGGameplayAbility::GetUnitCharacterFromActorInfo() const
{
	return Cast<ANGCharacterBase>(GetAvatarActorFromActorInfo());
}
