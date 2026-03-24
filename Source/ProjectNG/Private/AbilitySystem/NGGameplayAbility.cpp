// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGGameplayAbility.h"

#include "Character/NGUnitCharacter.h"

UNGGameplayAbility::UNGGameplayAbility()
{
	//인스턴싱 정책 (유닛마다 독립적인 쿨다운/데이터 관리를 위해 필수
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

ANGUnitCharacter* UNGGameplayAbility::GetUnitCharacterFromActorInfo() const
{
	return Cast<ANGUnitCharacter>(GetAvatarActorFromActorInfo());
}
