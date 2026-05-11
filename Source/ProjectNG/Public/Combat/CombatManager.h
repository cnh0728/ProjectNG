// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGCombatData.h"
#include "Pawn/NGPawnBase.h"
#include "CombatManager.generated.h"

class ANGPlayerController;
class ANGCharacterBase;
class AGridMapManager;
class ANGEnemyPawn;

UCLASS()
class PROJECTNG_API ACombatManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACombatManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void StartWave(APlayerController* PC);
	
protected:
	void SpawnEnemyTimerElapsed();

	bool SpawnEnemy();

public:
	void StartCombat(FCombatSettingData SettingData, APlayerController* PC);
	
	void PawnDied(ANGPawnBase* DeadPawn);

protected:
	
	void SetupCombat(FCombatSettingData SettingData);
	
	void FinishCombat();

	int32 CurrentEnemyCount = 0;
	int32 TargetKillCount = 10;
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> RequestingPlayerControllerCache;
	
	
	
public:
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FWaveData> WaveList;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	int32 CurrentWaveIndex;
	
private:
	FTimerHandle SpawnTimerHandle;
	int32 EnemiesSpawnedSoFar; //현재 웨이브에서 몇마리 소환됐는지
	
	UPROPERTY(EditDefaultsOnly, Category = "Setup")
	TSubclassOf<AGridMapManager> GridMapManagerClass;
};
