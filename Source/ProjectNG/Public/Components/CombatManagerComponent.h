// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatManagerComponent.generated.h"

class ANGPlayerState;
struct FWaveData;
struct FCombatSettingData;
class ANGPawnBase;
class AGridMapManager;

UCLASS()
class PROJECTNG_API UCombatManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UCombatManagerComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void StartCombat(FCombatSettingData SettingData, APlayerController* PC);
	void StartFight();

	void PawnDied(ANGPawnBase* DeadPawn);

	void FinishCombat();

	void ResetGrid();
protected:
	
	void SetupCombat(FCombatSettingData SettingData);

	int32 CurrentEnemyCount = 0;
	int32 TargetKillCount = 10;
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> RequestingPlayerControllerCache;
	
	UPROPERTY()
	TObjectPtr<ANGPlayerState> HomePS;
	
	UPROPERTY()
	TObjectPtr<ANGPlayerState> AwayPS;
	
private:
	
	
	FTimerHandle FightStartTimerHandle;
};
