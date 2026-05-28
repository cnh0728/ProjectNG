// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayCueInterface.h"
#include "Player/NGPlayerState.h"
#include "NGPawnBase.generated.h"

class UNGPathFindingComponent;
class UCapsuleComponent;
class UNGAttributeSet;
class ANGEnemyPawn;
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
class PROJECTNG_API ANGPawnBase : public APawn, public IAbilitySystemInterface, public IGameplayCueInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ANGPawnBase();
	
	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End IAbilitySystemInterface

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

	// virtual void ServerSideInit();
	// virtual void ClientSideInit();
	
protected:
	//캐싱 용도
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;
	
	virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	
protected:

	// virtual void ServerSideInit();
	// virtual void ClientSideInit();
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;
	void LookAtInterp(ANGPawnBase* Target, float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HPBarComponent; 
	
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
	void TransitionToState(EPawnState NewState);
	void OnApplyHardCrowdControl();
	void OnRemoveHardCrowdControl();
	void OnExitCurrentState(EPawnState RestState);
	void OnEnterNewState(EPawnState EnteringState);
	void SetNextGridPoint(FIntVector2 NewNextGridPoint);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, Category = "Visual")
	USkeletalMeshComponent* UnitMesh;
	
	//서버에서만 필요해서 Rep필요없음
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int32 OwnerIndex;
	
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
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
	
	virtual void InitializeAttributes();
	
public:
	virtual void Initialize(ANGPlayerState* PS);
	
	float GetMoveSpeed() const;
	
	UAnimMontage* GetAttackMontage() const;
	
	virtual void Die();
	
	bool IsDead();
	
	ANGPawnBase* GetCurrentTarget();
	void RestoreStates();

	void TurnPawnState(EPawnState InPawnState);
	
	bool IsSameTeam(uint32 OtherOwnerIndex) const { return OwnerIndex == OtherOwnerIndex; }
	
private:
	void UpdateHPBar();
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
