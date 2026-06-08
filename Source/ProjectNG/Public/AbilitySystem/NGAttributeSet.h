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
    // 리플리케이션(네트워크 동기화) 설정 함수
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
    
    // =========================================================================
    // Health & Mana (Vital Attributes)
    // =========================================================================
public:
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Health)
    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, MaxHealth)
    UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_Mana)
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Mana)
    UFUNCTION()
    void OnRep_Mana(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Vital", ReplicatedUsing = OnRep_MaxMana)
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, MaxMana)
    UFUNCTION()
    void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

    // =========================================================================
    // Stats (Primary Attributes)
    // =========================================================================
public:
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats", ReplicatedUsing = OnRep_Strength)
    FGameplayAttributeData Strength;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Strength)
    UFUNCTION()
    void OnRep_Strength(const FGameplayAttributeData& OldValue);

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats", ReplicatedUsing = OnRep_Intelligence)
    FGameplayAttributeData Intelligence;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Intelligence)
    UFUNCTION()
    void OnRep_Intelligence(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats", ReplicatedUsing = OnRep_Agility)
    FGameplayAttributeData Agility;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Agility)
    UFUNCTION()
    void OnRep_Agility(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats", ReplicatedUsing = OnRep_AttackDamage)
    FGameplayAttributeData AttackDamage;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, AttackDamage)
    
    UFUNCTION()
    void OnRep_AttackDamage(const FGameplayAttributeData& OldValue);
    
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Stats", ReplicatedUsing = OnRep_AbilityPower)
    FGameplayAttributeData AbilityPower;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, AbilityPower)
    
    UFUNCTION()
    void OnRep_AbilityPower(const FGameplayAttributeData& OldValue);
    
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackRange)
    FGameplayAttributeData AttackRange;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, AttackRange)
    UFUNCTION()
    void OnRep_AttackRange(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_AttackSpeed)
    FGameplayAttributeData AttackSpeed;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, AttackSpeed)
    UFUNCTION()
    void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);
    

    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_CriticalRate)
    FGameplayAttributeData CriticalRate;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, CriticalRate)
    
    UFUNCTION()
    void OnRep_CriticalRate(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_DodgeRate)
    FGameplayAttributeData DodgeRate;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, DodgeRate)
    
    UFUNCTION()
    void OnRep_DodgeRate(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_PhysicalDefense)
    FGameplayAttributeData PhysicalDefense;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, PhysicalDefense)
    
    UFUNCTION()
    void OnRep_PhysicalDefense(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_MagicDefense)
    FGameplayAttributeData MagicDefense;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, MagicDefense)
    
    UFUNCTION()
    void OnRep_MagicDefense(const FGameplayAttributeData& OldValue);
    
    // =========================================================================
    // Combat
    // =========================================================================
public:
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Defense", ReplicatedUsing = OnRep_Defense)
    FGameplayAttributeData Defense;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Defense)

    UFUNCTION()
    void OnRep_Defense(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_TargetCount)
    FGameplayAttributeData TargetCount;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, TargetCount)
    UFUNCTION()
    void OnRep_TargetCount(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_Income)
    FGameplayAttributeData Income;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Income)
    UFUNCTION()
    void OnRep_Income(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_MoveSpeed)
    FGameplayAttributeData MoveSpeed;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, MoveSpeed)
    UFUNCTION()
    void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_Star)
    FGameplayAttributeData Star;
    ATTRIBUTE_ACCESSORS(UNGAttributeSet, Star)
    UFUNCTION()
    void OnRep_Star(const FGameplayAttributeData& OldValue);
};
