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
		
		//여기서 언제 타겟 선택되는지 확인
		TWeakObjectPtr<AActor> TargetActor = Payload.TargetData.Get(0)->GetActors()[0];
		
		//발사체 클래스에 미리 정의된 함수가 있다면 호출
		if (Payload.TargetData.Num() > 0 && TargetActor != nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Target Detected: %s"), *TargetActor->GetName());

			if (ANGCharacterBase* NewTarget = Cast<ANGCharacterBase>(TargetActor.Get()))
			{
				//여기서 Target설정이 이상한 경우가 있음
				Projectile->SetTarget(NewTarget);
			}
		}else
		{
			UE_LOG(LogTemp, Error, TEXT("No Actor Detected"));
		}
		
	}
}
