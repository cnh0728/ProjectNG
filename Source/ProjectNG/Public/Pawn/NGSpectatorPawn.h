// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "NGSpectatorPawn.generated.h"

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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnRep_Controller() override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void InitHUD();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;
};
