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
	
	UFUNCTION()
	void FocusOnMyGrid();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnRep_Controller() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void PossessCamera(const FTransform& CameraTransform, const ANGPlayerState* PS);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void InitHUD();
	
	
	UFUNCTION()
	void OnRep_GridManager();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;
	
	FTimerHandle RetryTimerHandle;
	int32 RetryCount;

};
