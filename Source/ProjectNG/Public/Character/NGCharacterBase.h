// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "SelectableInterface.h"
#include "GameFramework/Character.h"
#include "NGCharacterBase.generated.h"

class UNGAttributeSet;
class UNGAbilitySystemComponent;

UCLASS(Abstract)
class PROJECTNG_API ANGCharacterBase : public ACharacter, public IAbilitySystemInterface, public ISelectableInterface
{
private:
	GENERATED_BODY()

public:
	ANGCharacterBase();
	
	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End IAbilitySystemInterface

	virtual void OnSelected_Implementation() override;
	virtual void OnDeselected_Implementation() override;
	

protected:
	/** 파생 클래스에서 GAS 초기화를 위한 로직을 작성 */
	virtual void InitAbilityActorInfo()	PURE_VIRTUAL(ANGPawnBase::InitAbilityActorInfo);

	// virtual void ServerSideInit();
	// virtual void ClientSideInit();
	
protected:
	//캐싱 용도
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "GAS|AttributeSet")
	TObjectPtr<UNGAttributeSet> AttributeSet;
};
