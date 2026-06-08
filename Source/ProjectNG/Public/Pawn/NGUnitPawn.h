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
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void Activate() override;
	
	virtual void Deactivate() override;
	
	virtual void OnSelected_Implementation() override;
	virtual void OnDeselected_Implementation() override;

	virtual void OnDrag_Implementation() override;
	virtual void OnUndrag_Implementation() override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void OnRep_PlayerState() override;

protected:
	virtual void InitAbilityActorInfo() override;
	
	virtual void InitializeAttributes() override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void ExecuteAttack();

	UFUNCTION(Server, Reliable)
	void SetDragTargetGridIndex(const FIntVector2& NewIndex);

	void SetPlacedGridIndex(const FIntVector2& NewIndex);
	
	FIntVector2 GetPlacedGridIndex();
	
	void SetCurrentGridIndex(const FIntVector2& NewIndex);
	
	void UpdateDecalRange();
	
	virtual void OnRep_PlayerState() override;
	
	void Initialize(ANGPlayerController* InController);
protected:
	virtual void OnAttackRangeChanged(const FOnAttributeChangeData& Data) override;
	
	UFUNCTION()
	void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void CheckAttackCondition();
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void InitializeAttributes() override;

private:
	UFUNCTION()
	void EquipWeapon(UNGWeaponData* NewWeaponData);
	
	virtual void InitAbilityActorInfo() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	float DragInterpSpeed = 15.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	float AcceptanceRadius;


};
