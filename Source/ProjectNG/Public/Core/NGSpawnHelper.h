// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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
	template<typename T>
	static T* SpawnPawn(UObject* WorldContextObject, TSubclassOf<ANGPawnBase> PawnClass,
								  FTransform SpawnTransform, AActor* Owner)
	{
		return Cast<T>(Internal_SpawnPawn(WorldContextObject, PawnClass, SpawnTransform, Owner));
	}
	
protected:
	static ANGPawnBase* Internal_SpawnPawn(UObject* WorldContextObject, TSubclassOf<ANGPawnBase> PawnClass,
	                              FTransform SpawnTransform, AActor* Owner);
};
