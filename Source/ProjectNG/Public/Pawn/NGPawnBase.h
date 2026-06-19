// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayCueInterface.h"
#include "Interface/Poolable.h"
#include "Player/NGPlayerState.h"
#include "NGPawnBase.generated.h"

struct FUnitAbilityData;
class UNGHPBarWidgetComponent;
class UNGPathFindingComponent;
class UCapsuleComponent;
class UNGAttributeSet;
class ANGCharacterBase;
class ANGEnemyPawn;
class USphereComponent;
class UWidgetComponent;
class UNGAbilitySystemComponent;

UENUM(BlueprintType)
enum class EPawnState : uint8
{
	None,
	Wait,
	Following,
	Combat,
	HardCrowdControl,
};

UCLASS()
class PROJECTNG_API ANGPawnBase : public APawn, public IAbilitySystemInterface, public IGameplayCueInterface, public IPoolable
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ANGPawnBase();
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void OnRep_PlayerState() override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	//~Begin IPoolable
	virtual void Activate() override;
	
	virtual void Deactivate() override;
	//~End IPoolable

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Activate();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Deactivate();
	
	virtual void Die();
	
	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End IAbilitySystemInterface

	virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return UnitMesh; }
	FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multi_PlayMontage(UAnimMontage* MontageToPlay);
	
	float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);
    
	void StopAnimMontage(UAnimMontage* AnimMontage);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	void HighlightRangeIndicator(FGridAddress PivotAddress) const;
	
protected:
	/** 파생 클래스에서 GAS 초기화를 위한 로직을 작성 */
	virtual void InitAbilityActorInfo()	PURE_VIRTUAL(ANGPawnBase::InitAbilityActorInfo);

	void LookAtInterp(ANGPawnBase* Target, float DeltaTime);
protected:
	//캐싱 용도
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UNGHPBarWidgetComponent> HPBarComponent; 
	
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	virtual void OnAttackRangeChanged(const FOnAttributeChangeData& Data);

	void VisualizePath();

	void CheckCombatState();
	void ConsiderTransitionState();
	
	void VisualizeFollowing(float DeltaTime);
	
	void OnReachedNextGrid();
	
	void ReFindPath();
	void FindNewTarget();
	void InitializeFindNewPath();

	bool IsCurrentTargetInRange() const;
	void CollectInRangeUnits(TArray<ANGPawnBase*>& OutEnemies);

	void ForceTransitionToState(EPawnState NewState);
	void OnApplyHardCrowdControl();
	void OnRemoveHardCrowdControl();
	void OnExitCurrentState(EPawnState RestState);
	void OnEnterNewState(EPawnState EnteringState);
	void SetNextGridPoint(FIntVector2 NewNextGridPoint);

protected:
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn|GameplayTag")
	FGameplayTag IdentificationTag;
	
	FGameplayTagContainer ActiveLooseTags;
	
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, Category = "Visual")
	USkeletalMeshComponent* UnitMesh;
	
	//서버에서만 필요해서 Rep필요없음
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int32 OwnerIndex;
	
	UPROPERTY(Replicated, BlueprintReadOnly, VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	EPawnState PawnState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ANGPawnBase> CurrentTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	FIntVector2 TargetLastIndex;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<FIntVector2> TargetPath;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int32 CurrentPathIndex;

	//무조건 SetNextGridPoint 함수를 사용해서 설정할것
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FIntVector2 NextGridPoint;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float SpeedScale;	
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float RotationInterpSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle AttackAbilitySpecHandle;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle CurrentWeaponAbilityHandle;
	
	FTimerHandle AttackCheckTimerHandle;
	
	FTimerHandle PredictGridReachingTimerHandle;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAttributeSet> AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UDataTable> DefaultAttributeTable;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FName UnitName;
	
	virtual void InitializeAttributes();
	
public:
	void TransitionToState(EPawnState NewState);
	
	virtual void Initialize(ANGPlayerState* PS);
	
	float GetMoveSpeed() const;
	
	UAnimMontage* GetAttackMontage() const;
	
	bool IsDead();
	
	ANGPawnBase* GetCurrentTarget();
	void RestoreStates();

	bool IsSameTeam(uint32 OtherOwnerIndex) const { return OwnerIndex == OtherOwnerIndex; }
	
	FGameplayTag GetIdentificationTag() const { return IdentificationTag; };
	
	void SetIdentificationTag(const FGameplayTag InIdentificationTag) { IdentificationTag = InIdentificationTag; }
	
	void SetUnitName(const FName& InUnitName) { UnitName = InUnitName; }
	FName GetUnitName() const { return UnitName; }
	
private:
	void UpdateHPBar();
	
	void InitAbilityData(const FUnitAbilityData& AbilityData);
	
	bool CanAddUnitOnCombatGrid(EGridType NewGridType) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DamagedMontage;
	
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;
	void GrantHoverState();
	void RemoveHoverState() const;
	
	uint8 bIsDrag : 1;
	uint8 bIsSelected : 1;
	
public:
	UFUNCTION(Server, Reliable)
	void Server_TryMoveGrid(const FVector& TargetLocation, FGridAddress GridAddress);
	UFUNCTION(Client, Reliable)
	void Client_RejectMove();

	void TryMoveTo(const FVector& TargetLocation);
	void TranslatePawnOnGrid(const FGridAddress& GridAddress);
	void SetPawnOnGrid(const FGridAddress& GridAddress);
	void UnSetPawnOnGrid(const FGridAddress& GridAddress) const;
	virtual void UpdateCurrentGridAddress(FGridAddress NewGridAddress);
	void UpdatePawnCurrentLocation(const FGridAddress& GridAddress);

	bool CanPlaceUnit(FGridMapBase& GridMap, FIntVector2 GridIndex);
	const FGridAddress& GetGridAddress() const { return CurrentGridAddress; };
	
	void ExecuteAttack();

protected:
	void CheckAttackCondition();

	UFUNCTION()
	void OnRep_CurrentGridAddress();
	void LookAt(ANGPawnBase* Target);

	//클라이언트 reject용
	UPROPERTY(EditDefaultsOnly, Category = "GridIndex", meta = (AllowPrivateAccess = "true"))
	FGridAddress PreGridAddress;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentGridAddress, EditDefaultsOnly, Category = "GridIndex", meta = (AllowPrivateAccess = "true"))
	FGridAddress CurrentGridAddress;
	
	UPROPERTY(VisibleAnywhere, Category = "PathFinding")
	UNGPathFindingComponent* PathFindingComponent;
};
