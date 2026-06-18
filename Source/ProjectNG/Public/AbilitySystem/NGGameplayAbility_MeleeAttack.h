// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGGameplayAbility_Attack.h"
#include "NGGameplayAbility_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API UNGGameplayAbility_MeleeAttack : public UNGGameplayAbility_Attack
{
	GENERATED_BODY()
	
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION()
	void OnAttackReceived(FGameplayEventData Payload);
	
	UPROPERTY()
	FGameplayEventData TossTargetData;
};
