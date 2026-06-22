// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h" //자식에서 공용으로 쓰기때문에 지우지 말 것
#include "NGAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

#define DEFAULT_REPLICATION_IMPLEMENTATION(ClassName, Name) \
void ClassName::OnRep_##Name(const FGameplayAttributeData& OldValue) \
{ GAMEPLAYATTRIBUTE_REPNOTIFY(ClassName, Name, OldValue); }

// 리플렉션이라서 매크로로 쓰면 에러남 매크로에 있는 UPROPERTY를 언리얼 헤더 툴이 인지하지 못한다는듯
// 즉 언리얼 헤더툴이 매크로가 풀리는 시간보다 더 빨리 작동해서 생기는 문제인듯?
// #define DEFINE_ATTRIBUTE(Name) \
// 	ATTRIBUTE_ACCESSORS(ThisClass, Name) \
// 	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_##Name, Category = "Attributes") \
// 	FGameplayAttributeData Name; \
// 	UFUNCTION() \
// 	void OnRep_##Name(const FGameplayAttributeData& OldValue); \

/**
 * 
 */
UCLASS()
class PROJECTNG_API UNGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
    
	
};
