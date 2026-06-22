// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Player/NGPlayerController.h"
#include "NGSpawnHelper.generated.h"

class ANGPawnBase;
class ANGUnitPawn;
/**
 * 
 */
UCLASS()
class PROJECTNG_API UNGSpawnHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:	
	template<typename T = ANGPawnBase>
	requires std::derived_from<T, ANGPawnBase>
	static T* SpawnPawn(UObject* WorldContextObject, TSubclassOf<ANGPawnBase> PawnClass,
								  FTransform SpawnTransform, AActor* Owner)
	{
		return Cast<T>(Internal_SpawnPawn(WorldContextObject, PawnClass, SpawnTransform, Owner));
	}
	
	static ANGUnitPawn* SpawnUnitPawn(ANGPlayerController* OwnerController, FName UnitName);
	static ANGUnitPawn* SpawnUnitPawnAtGrid(ANGPlayerController* OwnerController, FName UnitName,
	                                        const FGridAddress& SpawnGridAddress);
	
	static ANGEnemyPawn* SpawnEnemyPawn(ANGPlayerController* OwnerController, FEnemySpawnInfo EnemySpawnInfo);
		
protected:
	static ANGPawnBase* Internal_SpawnPawn(UObject* WorldContextObject, TSubclassOf<ANGPawnBase> PawnClass,
	                              FTransform SpawnTransform, AActor* Owner);
};
