// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NGGameState.generated.h"

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
};
