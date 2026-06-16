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
	
	static bool SpawnUnitPawn(ANGPlayerController* OwnerController, FName UnitName);
	static bool SpawnUnitPawnAtGrid(ANGPlayerController* OwnerController, FName UnitName, const FGridAddress& SpawnGridAddress);
	
protected:
	static ANGPawnBase* Internal_SpawnPawn(UObject* WorldContextObject, TSubclassOf<ANGPawnBase> PawnClass,
	                              FTransform SpawnTransform, AActor* Owner);
};
