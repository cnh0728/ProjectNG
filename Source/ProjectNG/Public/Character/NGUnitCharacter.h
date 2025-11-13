// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/NGCharacterBase.h"
#include "NGUnitCharacter.generated.h"

UCLASS()
class PROJECTNG_API ANGUnitCharacter : public ANGCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANGUnitCharacter();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	virtual void InitAbilityActorInfo() override;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
