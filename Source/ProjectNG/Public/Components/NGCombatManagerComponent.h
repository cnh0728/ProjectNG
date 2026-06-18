// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/NGEnemyDataAsset.h"
#include "NGCombatManagerComponent.generated.h"

struct FEnemySquadData;
class ANGPlayerController;
class ANGPawnBase;
class ANGPlayerState;
struct FCombatSettingData;

UENUM(BlueprintType)
enum class ECombatResult : uint8
{
	None = 0,
	Win = 1,
	Lose = 2,
	Draw = 3,
};

UCLASS()
class PROJECTNG_API UNGCombatManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UNGCombatManagerComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void StartCombat();
	void ReturnSpectatorHome(ANGPlayerState* AwayPlayer);
	void NotifyEndCombat(const ANGPlayerState* LoseEndPlayer);

	void NotifyPawnDied(ANGPawnBase* DeadPawn);

	void FinishCombat();
	void ClearCombatEndDatas();

	void ResetGrid(ANGPlayerState* PS);

	void EnqueueCombatPhase(ANGPlayerState* PS, const FEnemySquadData* CPUCombatData = nullptr);
	void StartCountingCombat();
	void MatchingCombatUser();

	void RequestSpawnSquadByPlayer(ANGPlayerController* RequestingPC, const FEnemySquadData& SquadData);
	
	void ProcessPlayerFlee(ANGPlayerController* PlayerController);
	
protected:
	//매개변수 의도된 복사
	void SetupCombat(const FCombatSettingData& SettingData);
	
	UPROPERTY()
	TArray<ANGPlayerState*> CombatPSMatchingQueue;

	UPROPERTY()
	TArray<FCombatSettingData> CombatDatas;
	
	UPROPERTY()
	TMap<ANGPlayerState*, ECombatResult> CombatResultDictionary;
	
	UPROPERTY()
	int32 FinishedCombatCount;
	
	UPROPERTY(EditAnywhere, meta=(AllowPrivateAccess = "true"))
	float FightWaitTime;
	
	FTimerHandle FightStartTimerHandle;
};
