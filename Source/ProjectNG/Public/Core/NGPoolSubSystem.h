// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Pawn/NGPawnBase.h"
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
struct FNGPawnList
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<ANGPawnBase*> FreePawnList;
};

UCLASS()
class PROJECTNG_API UNGPoolSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	ANGProjectile* AcquireProjectile(TSubclassOf<ANGProjectile> ProjectileClass, const FTransform& SpawnTransform, ANGPawnBase* Target);
	void ReleaseSegment(ANGProjectile* Projectile);
	
	ANGPawnBase* AcquirePawn(TSubclassOf<ANGPawnBase> PawnClass, const FTransform& SpawnTransform);
	void ReleaseSegment(ANGPawnBase* Pawn);
	
	void ReleaseDefault(AActor* InActor);
protected:
	UPROPERTY()
	TMap<TSubclassOf<ANGProjectile>, FNGProjectileList> ProjectilePools;
	
	UPROPERTY()
	TMap<TSubclassOf<ANGPawnBase>, FNGPawnList> PawnPools;
	
};
