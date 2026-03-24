// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "AbilitySystem/NGGameplayAbility_ProjectileAttack.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/NGCharacterBase.h"
#include "Combat/Weapon/NGProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

UNGGameplayAbility_ProjectileAttack::UNGGameplayAbility_ProjectileAttack()
{
}

void UNGGameplayAbility_ProjectileAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		FGameplayTag::RequestGameplayTag(FName("Event.Attack.Release"))
		);
	WaitEventTask->EventReceived.AddDynamic(this, &UNGGameplayAbility_ProjectileAttack::OnReleaseProjectile);
	WaitEventTask->ReadyForActivation();
	
}

void UNGGameplayAbility_ProjectileAttack::OnReleaseProjectile(FGameplayEventData Payload)
{
	if (!GetWorld() || !ProjectileClass) return;
	
	FVector SpawnLocation = GetUnitCharacterFromActorInfo()->GetActorLocation() + GetUnitCharacterFromActorInfo()->GetActorForwardVector() * 100.f;
	FRotator SpawnRotation = GetUnitCharacterFromActorInfo()->GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetUnitCharacterFromActorInfo();
	SpawnParams.Instigator = Cast<APawn>(GetUnitCharacterFromActorInfo());
	
	ANGProjectile* Projectile = GetWorld()->SpawnActor<ANGProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	//DamageEffectClass를 Instigator의 DamageEffect를 가져와서 넣어야하는거 아닌가?
	
	if (Projectile && DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		Projectile->SetSpecHandle(SpecHandle);
		
		//발사체 클래스에 미리 정의된 함수가 있다면 호출
		if (Payload.TargetData.Num() > 0 && Payload.TargetData.Get(0)->GetActors()[0] != nullptr)
		{
			if (ANGCharacterBase* NewTarget = Cast<ANGCharacterBase>(Payload.TargetData.Get(0)->GetActors()[0].Get()))
			{
				Projectile->SetTarget(NewTarget);
			}
		}
		
	}
}
