// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGGameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Pawn/NGPawnBase.h"

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
	
	ANGPawnBase* Unit = GetUnitPawnFromActorInfo();
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("UNGGameplayAbility::ActivateAbility - No Unit"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ANGPawnBase* Target = Unit->GetCurrentTarget();
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
		float AnimLength = MontageToPlay->GetPlayLength();
		float FinalPlayRate = AnimLength / CurrentAS;
		
		UAbilityTask_PlayMontageAndWait* AbilityTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TEXT("Anim"), MontageToPlay, FinalPlayRate);
		AbilityTask->OnCompleted.AddDynamic(this, &UNGGameplayAbility::OnMontageFinished);
		AbilityTask->OnInterrupted.AddDynamic(this, &UNGGameplayAbility::OnMontageFinished);
		AbilityTask->OnCancelled.AddDynamic(this, &UNGGameplayAbility::OnMontageFinished);
		AbilityTask->ReadyForActivation();
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("UNGGameplayAbility::ActivateAbility - No MontageToPlay"));
		
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

ANGPawnBase* UNGGameplayAbility::GetUnitPawnFromActorInfo() const
{
	return Cast<ANGPawnBase>(GetAvatarActorFromActorInfo());
}

void UNGGameplayAbility::OnMontageFinished()
{
	// UE_LOG(LogTemp, Log, TEXT("애니메이션이 완전히 끝나서 공격 락(태그)을 해제합니다."));
    
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}