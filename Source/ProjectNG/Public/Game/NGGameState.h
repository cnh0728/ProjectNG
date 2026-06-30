// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Map/NGMapTypes.h"
#include "NGGameState.generated.h"

class ANGPlayerState;
class AArena;
class UNGCombatManagerComponent;

UENUM(BlueprintType)
enum class EGameplayPhase : uint8
{
	None,
	TownSelection,
	NodeSelection,
	ActionPhase,
	TurnEnd
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapDataReadySignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnGameFlowChangedSignature, EGameplayPhase, CurrentPhase, int32, CurrentTurn, float, PhaseStartServerTime, float, PhaseDuration, float, RemainingTime);

/**
 * 
 */
UCLASS()
class PROJECTNG_API ANGGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ANGGameState();
	
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
		
	uint8 GetPlayerId(const ANGPlayerState* InPS);
	
	float GridMargin;
	
	UPROPERTY(ReplicatedUsing = OnRep_GameFlow, BlueprintReadOnly, Category = "Game|Turn")
	EGameplayPhase CurrentPhase = EGameplayPhase::None;

	UPROPERTY(ReplicatedUsing = OnRep_GameFlow, BlueprintReadOnly, Category = "Game|Turn")
	int32 CurrentTurn = 0;

	UPROPERTY(ReplicatedUsing = OnRep_GameFlow, BlueprintReadOnly, Category = "Game|Turn")
	float RemainingTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_GameFlow, BlueprintReadOnly, Category = "Game|Turn")
	float PhaseStartServerTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_GameFlow, BlueprintReadOnly, Category = "Game|Turn")
	float PhaseDuration = 0.f;
	
	UPROPERTY(BlueprintAssignable, Category = "Game|Turn")
	FOnGameFlowChangedSignature OnGameFlowChanged;
	
	UFUNCTION()
	void OnRep_GameFlow();
	
	void BroadcastGameFlowChanged();

	void SetGameFlow(EGameplayPhase NewPhase, float NewPhaseDuration);

	UFUNCTION(BlueprintPure, Category = "Game|Turn")
	float GetRemainingTimeByServerClock() const;

	UFUNCTION(BlueprintPure, Category = "Game|Turn")
	float GetPhasePercentRemaining() const;

public:
	// 맵 데이터
	UPROPERTY(ReplicatedUsing = OnRep_MapNodes, BlueprintReadOnly, Category = "Game|Map")
	TArray<FMapNodeData> MapNodes;

	UFUNCTION()
	void OnRep_MapNodes();

	// 맵 동기화 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Game|Map")
	FOnMapDataReadySignature OnMapDataReady;

	// 서버 전용: 맵 데이터 세팅
	void SetMapNodes(const TArray<FMapNodeData>& InNodes);
};
