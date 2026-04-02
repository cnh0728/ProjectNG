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
struct FNGProjectileList
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<ANGProjectile*> FreeProjectileList;
};

USTRUCT()
struct FNGCharacterList
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<ANGCharacterBase*> FreeCharacterList;
};

UCLASS()
class PROJECTNG_API UNGPoolSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	ANGProjectile* AcquireProjectile(TSubclassOf<ANGProjectile> ProjectileClass, const FTransform& SpawnTransform, ANGCharacterBase* Target);
	void ReleaseProjectile(ANGProjectile* Projectile);
	
	ANGCharacterBase* AcquireCharacter(TSubclassOf<ANGCharacterBase> CharacterClass, const FTransform& SpawnTransform);
	void ReleaseCharacter(ANGCharacterBase* Character);
	
protected:
	UPROPERTY()
	TMap<TSubclassOf<ANGProjectile>, FNGProjectileList> ProjectilePools;
	
	UPROPERTY()
	TMap<TSubclassOf<ANGCharacterBase>, FNGCharacterList> CharacterPools;
	
};
