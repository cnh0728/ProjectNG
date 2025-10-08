// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "NGPawnBase.generated.h"

UCLASS()
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
	UPROPERTY(BlueprintReadOnly, Category = "AbilitySystemComponent")
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;
};
