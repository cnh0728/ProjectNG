// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGPoolSubSystem.h"

#include "Combat/Weapon/NGProjectile.h"

ANGProjectile* UNGPoolSubSystem::AcquireProjectile(TSubclassOf<ANGProjectile> ProjectileClass,
                                                   const FTransform& SpawnTransform, ANGPawnBase* Target)
{
	if (!ProjectileClass)	return nullptr;
	
	FNGProjectileList& Pool = ProjectilePools.FindOrAdd(ProjectileClass);
	ANGProjectile* Projectile;
	bool bNewSpawnProjectile = false;
	
	//여유분이 있으면 반환
	if (Pool.FreeProjectileList.Num() > 0)
	{
		Projectile = Pool.FreeProjectileList.Pop();
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


ANGPawnBase* UNGPoolSubSystem::AcquirePawn(TSubclassOf<ANGPawnBase> PawnClass,
                                           const FTransform& SpawnTransform, const FActorSpawnParameters& SpawnParams)
{
	if (!PawnClass)	return nullptr;
	
	FNGPawnList& Pool = PawnPools.FindOrAdd(PawnClass);
	
	ANGPawnBase* Pawn;
	
	//여유분이 있으면 반환
	if (Pool.FreePawnList.Num() > 0)
	{
		Pawn = Pool.FreePawnList.Pop();
	}
	else
	{
		Pawn = GetWorld()->SpawnActor<ANGPawnBase>(PawnClass, SpawnTransform, SpawnParams);
	}
	
	return Pawn;
}

void UNGPoolSubSystem::ReleaseDefault(AActor* InActor)
{
	check(InActor);
	
	InActor->SetActorHiddenInGame(true);
	InActor->SetActorEnableCollision(false);
	InActor->SetActorTickEnabled(false);
}

void UNGPoolSubSystem::ReleaseSegment(ANGPawnBase* Pawn)
{
	//TODO: release는 굉장히 유사해서 Template으로 하고 싶었는데 Pool을 통일해버리면 드롭리스트가 전부나와서 지저분해져서 걍 분리 
	if (!Pawn)	return;
	
	ReleaseDefault(Pawn);
	
	FNGPawnList& Pool = PawnPools.FindOrAdd(Pawn->GetClass());
	Pool.FreePawnList.Push(Pawn);
}

void UNGPoolSubSystem::ReleaseSegment(ANGProjectile* Projectile)
{
	if (!Projectile)	return;
	
	ReleaseDefault(Projectile);
	
	FNGProjectileList& Pool = ProjectilePools.FindOrAdd(Projectile->GetClass());
	Pool.FreeProjectileList.Push(Projectile);
}