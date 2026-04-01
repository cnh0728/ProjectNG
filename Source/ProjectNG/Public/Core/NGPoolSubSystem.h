// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/NGCharacterBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "NGPoolSubSystem.generated.h"

/**
 * 
 */

class ANGProjectile;

USTRUCT()
struct FProjectileList
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<ANGProjectile*> FreeList;
};

UCLASS()
class PROJECTNG_API UNGPoolSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	ANGProjectile* AcquireProjectile(TSubclassOf<ANGProjectile> ProjectileClass, const FTransform& SpawnTransform, ANGCharacterBase* Target);
	void ReleaseProjectile(ANGProjectile* Projectile);
	
protected:
	UPROPERTY()
	TMap<TSubclassOf<ANGProjectile>, FProjectileList> ProjectilePools;
	
	
};
