// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NGGameState.generated.h"

class ANGPlayerState;
class AGridMapManager;
class UCombatManagerComponent;
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
		
	uint8 RegisterPlayer(ANGPlayerState* InPS);
	
	float GridMargin;
	
	UCombatManagerComponent* GetCombatManagerComponent() { return CombatManagerComponent; };	
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Users")
	TArray<ANGPlayerState*> PlayerStates;
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	TObjectPtr<UCombatManagerComponent> CombatManagerComponent;
};
