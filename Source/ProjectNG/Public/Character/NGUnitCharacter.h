// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Character/NGCharacterBase.h"
#include "NGUnitCharacter.generated.h"

class UNGWeaponData;
class UGameplayAbility;
class ANGEnemyCharacter;
struct FOnAttributeChangeData;
class USphereComponent;
class AGridMapManager;
class UGameplayEffect;

UCLASS()
class PROJECTNG_API ANGUnitCharacter : public ANGCharacterBase, public ISelectableInterface
{
	GENERATED_BODY()

public:
	ANGUnitCharacter();
	
	virtual void OnSelected_Implementation() override;
	virtual void OnDeselected_Implementation() override;
	void ShowRangeIndicator(bool bVisible) const;

	virtual void OnDrag_Implementation() override;
	virtual void OnUndrag_Implementation() override;
	
	virtual void Activate() override;
	virtual void Deactivate() override;
	
protected:
	virtual void InitializeAttributes() override;
	virtual void InitAbilityActorInfo() override;
	
public:
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	void ExecuteAttack();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void SetDragTargetGridIndex(const TOptional<FIntVector2>& NewIndex);
	
	void SetPlacedGridIndex(const FIntVector2& NewIndex);
	TOptional<FIntVector2> GetPlacedGridIndex();
	
	void RefreshCache();
	
	void UpdateDecalRange();
	
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
	uint8 bIsDragMoving : 1;
	uint8 bIsSelected : 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "GridIndex")
	TOptional<FIntVector2> PlacedGridIndex;
	
	UPROPERTY()
	AGridMapManager* MapManagerCache;
};
