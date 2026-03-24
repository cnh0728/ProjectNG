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
	// Sets default values for this character's properties
	ANGUnitCharacter();
	
	virtual void OnSelected_Implementation() override;
	virtual void OnDeselected_Implementation() override;
	void ShowRangeIndicator(bool bVisible) const;

	virtual void OnDrag_Implementation() override;
	virtual void OnUndrag_Implementation() override;
	
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

protected:
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	TObjectPtr<USphereComponent> DetectionSphere;
	
	//Queue로 하고싶은데
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<ANGEnemyCharacter*> DetectedTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ANGEnemyCharacter> CurrentTarget;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float RotationInterpSpeed = 10.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "RangeDecal")
	TObjectPtr<UDecalComponent> RangeDecal;
	
	UPROPERTY(EditAnywhere, Category = "RangeDecal")
	TObjectPtr<UMaterialInterface> RangeMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle AttackAbilitySpecHandle;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle CurrentWeaponAbilityHandle;
	
	FTimerHandle AttackCheckTimerHandle;
	
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
