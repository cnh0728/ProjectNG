// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGGameplayAbility_MeleeAttack.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

class UAbilityTask_WaitGameplayEvent;

void UNGGameplayAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                     const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                     const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag(TEXT("Event.Attack.Melee")));
	EventTask->EventReceived.AddDynamic(this, &UNGGameplayAbility_MeleeAttack::OnAttackReceived);
	EventTask->ReadyForActivation();
}

void UNGGameplayAbility_MeleeAttack::OnAttackReceived(FGameplayEventData Payload)
{
	UE_LOG(LogTemp, Log, TEXT("OnAttackHitReceived"));
	
	if (DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			TArray<TWeakObjectPtr<AActor>> TargetActors = Payload.TargetData.Get(0)->GetActors();
			if (TargetActors.Num() > 0 && TargetActors[0].IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("Target Actor Name: %s"), *TargetActors[0]->GetName());
			}
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, Payload.TargetData);
		}
	}
}
