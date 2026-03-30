// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayCueInterface.h"
#include "SelectableInterface.h"
#include "GameFramework/Character.h"
#include "NGCharacterBase.generated.h"

struct FOnAttributeChangeData;
class UWidgetComponent;
class UAttributeSet;
class UNGAttributeSet;
class UNGAbilitySystemComponent;

UCLASS(Abstract)
class PROJECTNG_API ANGCharacterBase : public ACharacter, public IAbilitySystemInterface, public IGameplayCueInterface
{
private:
	GENERATED_BODY()

public:
	ANGCharacterBase();
	
	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End IAbilitySystemInterface

	virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	
protected:
	/** 파생 클래스에서 GAS 초기화를 위한 로직을 작성 */
	virtual void InitAbilityActorInfo()	PURE_VIRTUAL(ANGPawnBase::InitAbilityActorInfo);

	// virtual void ServerSideInit();
	// virtual void ClientSideInit();
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HPBarComponent; 
	
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	virtual void OnAttackRangeChanged(const FOnAttributeChangeData& Data);
	
	virtual void PlayHitReaction();
	
protected:
	//캐싱 용도
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<const UNGAttributeSet> AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UDataTable> DefaultAttributeTable;
	
	virtual void InitializeAttributes();
	
public:
	FVector GetHalfCapsule() const;
	
	UAnimMontage* GetAttackMontage() const;
	
	virtual void Die();
	
	bool IsDead();
	
private:
	void UpdateHPBar();
	
protected:
	FVector LocationOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DamagedMontage;
};
