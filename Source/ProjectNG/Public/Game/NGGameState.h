// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NGGameState.generated.h"

class ANGUnitPawn;
class AGridMapManager;
enum class EUnitTier : uint8;
class ACombatManager;
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
		
	uint8 AddGridMapManager(AGridMapManager* InitGridMap);
	
	AGridMapManager* GetGridMapManager(uint8 Id){ return GridMapManagers[Id]; }
	
	float GridMargin;
	
protected:

	
	UPROPERTY(Replicated, Transient, BlueprintReadOnly, Category = "Managers")
	TArray<AGridMapManager*> GridMapManagers;
	
};
