// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGPoolSubSystem.h"

#include "Combat/Weapon/NGProjectile.h"

ANGProjectile* UNGPoolSubSystem::AcquireProjectile(TSubclassOf<ANGProjectile> ProjectileClass,
                                                   const FTransform& SpawnTransform, ANGCharacterBase* Target)
{
	if (!ProjectileClass)	return nullptr;
	
	FProjectileList& Pool = ProjectilePools.FindOrAdd(ProjectileClass);
	ANGProjectile* Projectile;
	bool bNewSpawnProjectile = false;
	
	//여유분이 있으면 반환
	if (Pool.FreeList.Num() > 0)
	{
		Projectile = Pool.FreeList.Pop();
	}
	else
	{
		Projectile = GetWorld()->SpawnActorDeferred<ANGProjectile>(ProjectileClass, SpawnTransform);
		bNewSpawnProjectile = true;
	}
	
	if (Projectile)
	{
		Projectile->SetActorTransform(SpawnTransform);
		
		Projectile->SetActorHiddenInGame(false);
		Projectile->SetActorEnableCollision(true);
		Projectile->SetActorTickEnabled(true);
		
		Projectile->SetTarget(Target);
		
		if (!Projectile->HasActorBegunPlay() && bNewSpawnProjectile)
		{
			Projectile->FinishSpawning(SpawnTransform);
		}
	}
	
	return Projectile;
}

void UNGPoolSubSystem::ReleaseProjectile(ANGProjectile* Projectile)
{
	if (!Projectile)	return;
	
	Projectile->SetActorHiddenInGame(true);
	Projectile->SetActorEnableCollision(false);
	Projectile->SetActorTickEnabled(false);
	
	FProjectileList& Pool = ProjectilePools.FindOrAdd(Projectile->GetClass());
	Pool.FreeList.Push(Projectile);
}
