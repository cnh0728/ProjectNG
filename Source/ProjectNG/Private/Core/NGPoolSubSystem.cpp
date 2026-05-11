// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGPoolSubSystem.h"

#include "Combat/Weapon/NGProjectile.h"
#include "Game/NGUnitDataManager.h"

ANGProjectile* UNGPoolSubSystem::AcquireProjectile(TSubclassOf<ANGProjectile> ProjectileClass,
                                                   const FTransform& SpawnTransform, ANGCharacterBase* Target)
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


ANGCharacterBase* UNGPoolSubSystem::AcquireCharacter(TSubclassOf<ANGCharacterBase> CharacterClass,
	const FTransform& SpawnTransform)
{
	if (!CharacterClass)	return nullptr;
	
	FNGCharacterList& Pool = CharacterPools.FindOrAdd(CharacterClass);
	
	ANGCharacterBase* Character;
	
	//여유분이 있으면 반환
	if (Pool.FreeCharacterList.Num() > 0)
	{
		Character = Pool.FreeCharacterList.Pop();
		Character->SetActorTransform(SpawnTransform);
	}
	else
	{
		Character = GetWorld()->SpawnActor<ANGCharacterBase>(CharacterClass, SpawnTransform);
	}
	
	Character->Activate();
	
	return Character;
}

void UNGPoolSubSystem::ReleaseDefault(AActor* InActor)
{
	check(InActor);
	
	InActor->SetActorHiddenInGame(true);
	InActor->SetActorEnableCollision(false);
	InActor->SetActorTickEnabled(false);
}

void UNGPoolSubSystem::ReleaseSegment(ANGCharacterBase* Character)
{
	//TODO: release는 굉장히 유사해서 Template으로 하고 싶었는데 Pool을 통일해버리면 드롭리스트가 전부나와서 지저분해져서 걍 분리 
	if (!Character)	return;
	
	ReleaseDefault(Character);
	
	FNGCharacterList& Pool = CharacterPools.FindOrAdd(Character->GetClass());
	Pool.FreeCharacterList.Push(Character);
}

void UNGPoolSubSystem::ReleaseSegment(ANGProjectile* Projectile)
{
	if (!Projectile)	return;
	
	ReleaseDefault(Projectile);
	
	FNGProjectileList& Pool = ProjectilePools.FindOrAdd(Projectile->GetClass());
	Pool.FreeProjectileList.Push(Projectile);
}