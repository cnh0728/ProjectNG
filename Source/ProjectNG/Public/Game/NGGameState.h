// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Map/NGMapTypes.h"
#include "NGGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapDataReadySignature);

class ANGPlayerState;
class AArena;
class UNGCombatManagerComponent;

UENUM(BlueprintType)
enum class EGameplayPhase : uint8
{
	None,
	NodeSelection,
	ActionPhase,
	TurnEnd
};

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
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game|Turn")
	EGameplayPhase CurrentPhase = EGameplayPhase::None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game|Turn")
	int32 CurrentTurn = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game|Turn")
	float RemainingTime = 0.f;

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
