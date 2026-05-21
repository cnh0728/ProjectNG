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
class USphereComponent;
class UWidgetComponent;
class UNGPoolableComponent;
class UNGAbilitySystemComponent;

UENUM(BlueprintType)
enum class EPawnState : uint8
{
	None,
	Wait,
	Combat,
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
	
	//Queue로 하고싶은데
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<ANGPawnBase*> InRangeTarget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<ANGPawnBase> CurrentTarget;
	
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
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UNGAttributeSet> AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|AbilitySystemComponent")
	TObjectPtr<UDataTable> DefaultAttributeTable;
	
	virtual void InitializeAttributes();
	
public:
	virtual void Initialize(ANGPlayerState* PS);
	
	FVector GetHalfCapsule() const;
	
	UAnimMontage* GetAttackMontage() const;
	
	virtual void Die();
	
	bool IsDead();
	
	ANGPawnBase* GetCurrentTarget();
	
	void TurnPawnState(EPawnState InPawnState);
	
	bool IsSameTeam(uint32 OtherOwnerIndex) const { return OwnerIndex == OtherOwnerIndex; }
	
private:
	void UpdateHPBar();
	
protected:
	// 모델링 위치 조정용으로 더해주기
	FVector LocationOffset;
	
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
	const FGridAddress& GetGridAddress() const { return PlacedGridAddress; };
	
	void ExecuteAttack();

protected:
	void CheckAttackCondition();

	//클라이언트 reject용
	UPROPERTY(EditDefaultsOnly, Category = "GridIndex", meta = (AllowPrivateAccess = "true"))
	FGridAddress PrePlacedGridAddress;
	
	UPROPERTY(Replicated, EditDefaultsOnly, Category = "GridIndex", meta = (AllowPrivateAccess = "true"))
	FGridAddress PlacedGridAddress;
	
	UPROPERTY(VisibleAnywhere, Category = "PathFinding")
	UNGPathFindingComponent* PathFindingComponent;
};
