// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGGameplayAbility_Attack.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NGPawnAttributeSet.h"

void UNGGameplayAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ANGPawnBase* Unit = GetUnitPawnFromActorInfo();
	if (!Unit)
	{
		UE_LOG(LogTemp, Warning, TEXT("UNGGameplayAbility_Attack::ActivateAbility - No Unit"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	TossTargetData = *TriggerEventData;
}


void UNGGameplayAbility_Attack::RegerateMana(const ANGPawnBase* Unit) const
{
	if (UNGAbilitySystemComponent* AbilitySystemComponent = Unit->GetNGAbilitySystemComponent())
	{
		if (const UNGPawnAttributeSet* AttributeSet = AbilitySystemComponent->GetSet<UNGPawnAttributeSet>())
		{
			float ManaRegenValue = AttributeSet->GetManaRegeneration();

			if (ManaRegenValue > 0.f)
			{
				UE_LOG(LogTemp, Log, TEXT("%s: RegenerateMana - %f"), *Unit->GetName(), ManaRegenValue);
				
				AbilitySystemComponent->ApplyModToAttribute(
				UNGPawnAttributeSet::GetManaAttribute(),
				EGameplayModOp::Additive,
				ManaRegenValue);
			}
		}
	}
}
