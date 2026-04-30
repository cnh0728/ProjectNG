// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SelectableInterface.h"
#include "Pawn/NGPawnBase.h"
#include "NGUnitPawn.generated.h"

class ANGPlayerController;
struct FOnAttributeChangeData;

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
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void InitializeAttributes() override;
	
private:
	virtual void InitAbilityActorInfo() override;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void ExecuteAttack();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
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
	
	UFUNCTION()
	void EquipWeapon(UNGWeaponData* NewWeaponData);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	float DragInterpSpeed = 15.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Drag Drop")
	float AcceptanceRadius;

	uint8 bIsGrabbed : 1;
	uint8 bIsSelected : 1;
	
	UPROPERTY(Replicated)
	uint8 bIsDragMoving : 1;

	UPROPERTY()
	uint8 bIsOnField : 1;
	
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "GridIndex")
	FIntVector2 PlacedGridIndex;
	
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "GridIndex")
	FIntVector2 CurrentGridIndex;
	
	UPROPERTY(Transient)
	TObjectPtr<ANGPlayerController> OwnerController;
};
