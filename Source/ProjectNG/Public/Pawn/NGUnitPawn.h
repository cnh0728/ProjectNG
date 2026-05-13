// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SelectableInterface.h"
#include "Combat/Grid/Grid.h"
#include "Pawn/NGPawnBase.h"
#include "NGUnitPawn.generated.h"

class UNGWeaponData;

UCLASS()
class PROJECTNG_API ANGUnitPawn : public ANGPawnBase, public ISelectableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANGUnitPawn();
	
	virtual void OnSelected_Implementation() override;
	virtual void OnDeselected_Implementation() override;
	void ShowRangeIndicator(bool bVisible) const;

	virtual void OnDrag_Implementation() override;
	virtual void OnUndrag_Implementation() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void OnRep_PlayerState() override;

	virtual void UpdatePlacedGridInfo(FGridAddress NewGridAddress) override;

	void UpdateDecalRange();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void InitializeAttributes() override;
	
	UFUNCTION()
	void EquipWeapon(UNGWeaponData* NewWeaponData);
private:
	virtual void InitAbilityActorInfo() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	float DragInterpSpeed = 15.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	float AcceptanceRadius;
	
	uint8 bIsGrabbed : 1;
	uint8 bIsSelected : 1;
	
	UPROPERTY(Replicated)
	uint8 bIsDragMoving : 1;

};
