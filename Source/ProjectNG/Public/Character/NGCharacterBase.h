// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayCueInterface.h"
#include "SelectableInterface.h"
#include "GameFramework/Character.h"
#include "NGCharacterBase.generated.h"

class USphereComponent;
class ANGEnemyCharacter;
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
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<USphereComponent> DetectionSphere;
	
	//Queue로 하고싶은데
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<ANGEnemyCharacter*> DetectedTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ANGCharacterBase> CurrentTarget;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float RotationInterpSpeed = 10.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "RangeDecal")
	TObjectPtr<UDecalComponent> RangeDecal;
	
	UPROPERTY(EditAnywhere, Category = "RangeDecal")
	TObjectPtr<UMaterialInterface> RangeMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle AttackAbilitySpecHandle;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle CurrentWeaponAbilityHandle;
	
	FTimerHandle AttackCheckTimerHandle;
	
protected:
	//캐싱 용도
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAttributeSet> AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UDataTable> DefaultAttributeTable;
	
	virtual void InitializeAttributes();
	
public:
	FVector GetHalfCapsule() const;
	
	UAnimMontage* GetAttackMontage() const;
	
	virtual void Die();
	
	bool IsDead();
	
	ANGCharacterBase* GetCurrentTarget();
	
private:
	void UpdateHPBar();
	
protected:
	FVector LocationOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DamagedMontage;
};
