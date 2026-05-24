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
class UNGPoolableComponent;
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
	
	//TODO: 디버깅용 실제론 있어서 안되는 함수
	void SetOwnerIndex(int32 NewOwnerIndex) { OwnerIndex = NewOwnerIndex; }
	
protected:
	/** 파생 클래스에서 GAS 초기화를 위한 로직을 작성 */
	virtual void InitAbilityActorInfo()	PURE_VIRTUAL(ANGPawnBase::InitAbilityActorInfo);

	// virtual void ServerSideInit();
	// virtual void ClientSideInit();
	
protected:
	//캐싱 용도
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pool", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UNGPoolableComponent> PoolController;
	
	virtual void HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	
	virtual void OnRep_PlayerState() override;
	virtual void PossessedBy(AController* NewController) override;
	
protected:

	// virtual void ServerSideInit();
	// virtual void ClientSideInit();
	
	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HPBarComponent; 
	
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	virtual void OnAttackRangeChanged(const FOnAttributeChangeData& Data);

	void VisualizePath();

	void CheckCombatState();
	void ConsiderTransitionState();
	
	void VisualizeGridMovement(float DeltaTime);
	
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
	
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	int32 CurrentPathIndex;

	//무조건 SetNextGridPoint 함수를 사용해서 설정할것
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FIntVector2 NextGridPoint;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float SpeedScale;	
	
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float RotationInterpSpeed;
	
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
	
	void TurnPawnState(EPawnState InPawnState);
	
	bool IsSameTeam(uint32 OtherOwnerIndex) const { return OwnerIndex == OtherOwnerIndex; }
	
private:
	void UpdateHPBar();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DamagedMontage;
	
	
public:
	UFUNCTION(Server, Reliable)
	void Server_MoveGrid(const FVector& TargetLocation, FGridAddress GridAddress);
	UFUNCTION(Client, Reliable)
	void Client_RejectMove();

	void MoveTo(const FVector& TargetLocation);
	void MovePawnOnGrid(const FGridAddress& GridAddress);
	void SetPawnOnGrid(const FGridAddress& GridAddress);
	void UnSetPawnOnGrid(const FGridAddress& GridAddress) const;
	virtual void UpdatePlacedGridInfo(FGridAddress NewGridAddress);
	
	bool CanPlaceUnit(FGridMapBase& GridMap, FIntVector2 GridIndex);
	EGridType GetCurrentGridType(const FVector& TargetLocation) const;
	const FGridAddress& GetGridAddress() const { return CurrentGridAddress; };
	
	void ExecuteAttack();

protected:
	void CheckAttackCondition();

	//클라이언트 reject용
	UPROPERTY(EditDefaultsOnly, Category = "GridIndex", meta = (AllowPrivateAccess = "true"))
	FGridAddress PreGridAddress;
	
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "GridIndex", meta = (AllowPrivateAccess = "true"))
	FGridAddress CurrentGridAddress;
	
	UPROPERTY(VisibleAnywhere, Category = "PathFinding")
	UNGPathFindingComponent* PathFindingComponent;
};
