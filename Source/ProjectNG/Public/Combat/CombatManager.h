// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatManager.generated.h"

class AGridMapManager;
class ANGEnemyCharacter;

USTRUCT(Blueprintable)
struct FWaveData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ANGEnemyCharacter> EnemyClass;
	
	UPROPERTY(EditAnywhere)
	int32 EnemyCount = 10;
	
	UPROPERTY(EditAnywhere)
	float SpawnInterval = 1.0f; //스폰 간격
	
	UPROPERTY(EditAnywhere)
	float SpawnRandomDevation = 1.0f; //간격에서 랜덤 간극
};

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
	void StartWave();
	
protected:
	void SpawnEnemyTimerElapsed();
	
	void SpawnEnemy();

	void UpdateGridManagerCache();

	bool IsPossibleSpawnCharacter(AGridMapManager* MapManager) const;
public:
	bool SpawnUnitCharacter(FName UnitName) const;
	
public:
	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FWaveData> WaveList;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	int32 CurrentWaveIndex;
	
private:
	FTimerHandle SpawnTimerHandle;
	int32 EnemiesSpawnedSoFar; //현재 웨이브에서 몇마리 소환됐는지
	
	UPROPERTY()
	TObjectPtr<AGridMapManager> GridMapManagerCache; //맵 매니저 캐시
};
