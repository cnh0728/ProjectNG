// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Pawn/NGPawnBase.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "Core/NGPoolableComponent.h"

ANGPawnBase::ANGPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;		//네트워크 복제 활성화
	SetReplicatingMovement(true);	//위치 속도 복제 활성화
	
	PoolController = CreateDefaultSubobject<UNGPoolableComponent>(FName("PoolController"));
	
	AbilitySystemComponent = CreateDefaultSubobject<UNGAbilitySystemComponent>(TEXT("Ability System Component"));
}

UAbilitySystemComponent* ANGPawnBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
