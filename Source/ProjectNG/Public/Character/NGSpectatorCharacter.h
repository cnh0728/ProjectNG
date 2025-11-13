// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "NGCharacterBase.h"
#include "NGSpectatorCharacter.generated.h"

class UCameraComponent;


/**
 * 게임 안에서 관전자 역할만 담당하는 Player Pawn
 */
UCLASS()
class PROJECTNG_API ANGSpectatorCharacter : public ANGCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ANGSpectatorCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** PlayerPawn에서 GAS Component를 초기화 합니다. */
	virtual void InitAbilityActorInfo() override;

protected:

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;


};
