// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "NGAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// 이걸로 선언하고 cpp파일에서 1. GetLifetime내부 DOREP호출이랑 2. OnRep함수 구현
#define DEFINE_ATTRIBUTE(Name) \
	ATTRIBUTE_ACCESSORS(ThisClass, Name) \
	UPROPERTY(ReplicatedUsing = OnRep_##Name) \
	FGameplayAttributeData Name; \
	UFUNCTION() \
	void OnRep_##Name(const FGameplayAttributeData& OldValue) const; \

/**
 * 
 */

UCLASS()
class PROJECTNG_API UNGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	DEFINE_ATTRIBUTE(Health)
	DEFINE_ATTRIBUTE(MaxHealth)
	DEFINE_ATTRIBUTE(Mana)
	DEFINE_ATTRIBUTE(MaxMana)
	
	DEFINE_ATTRIBUTE(AttackDamage)
	DEFINE_ATTRIBUTE(AttackSpeed)
	DEFINE_ATTRIBUTE(Defense)
	DEFINE_ATTRIBUTE(Income)
	
};
