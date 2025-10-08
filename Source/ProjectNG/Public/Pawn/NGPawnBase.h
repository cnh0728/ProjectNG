// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "NGPawnBase.generated.h"


UCLASS(Abstract)
class PROJECTNG_API ANGPawnBase : public APawn, public IAbilitySystemInterface
{
private:
	GENERATED_BODY()

public:
	ANGPawnBase();

	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	//~End IAbilitySystemInterface

protected:
	/** 파생 클래스에서 GAS 초기화를 위한 로직을 작성 */
	virtual void InitAbilityActorInfo()	PURE_VIRTUAL(ANGPawnBase::InitAbilityActorInfo);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "AbilitySystemComponent")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
