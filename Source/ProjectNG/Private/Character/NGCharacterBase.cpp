// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Character/NGCharacterBase.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"


ANGCharacterBase::ANGCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

UAbilitySystemComponent* ANGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ANGCharacterBase::OnSelected_Implementation()
{
	ISelectableInterface::OnSelected_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnSelected"));
}

void ANGCharacterBase::OnDeselected_Implementation()
{
	ISelectableInterface::OnDeselected_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnDeSelected"));
}

