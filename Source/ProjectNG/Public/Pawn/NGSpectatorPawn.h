// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "Player/NGPlayerState.h"
#include "NGSpectatorPawn.generated.h"

class AArena;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PROJECTNG_API ANGSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ANGSpectatorPawn();

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;

	virtual void OnRep_Controller() override;

	void PossessCamera(const FTransform& CameraTransform);
	
	UFUNCTION(Client, Reliable)
	void Client_PossessCamera(const FTransform& CameraTransform);

protected:
	void InitHUD();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;
	
};
