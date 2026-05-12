// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGSpawnHelper.h"

#include "Components/CapsuleComponent.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGPoolSubSystem.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"

bool UNGSpawnHelper::SpawnUnitPawn(ANGPlayerController* OwnerController, FName UnitName)
{
	ANGPlayerState* PS = Cast<ANGPlayerController>(OwnerController)->GetPlayerState<ANGPlayerState>();
	
	FQuadGridMap& WaitGridMap = PS->GetWaitGridMap();
	
	TOptional<FIntVector2> EmptyGridIndex = WaitGridMap.GetEmptyGridIndex();
	
	if (!WaitGridMap.IsPossibleSpawnPawn())		return false;

	UNGPocketComponent* Pocket = PS->GetPlayerPocket();
	if (!Pocket)	return false;
	
	UWorld* World = OwnerController->GetWorld();
	if (!World)	return false;
	
	ANGInGameMode* GM = World->GetAuthGameMode<ANGInGameMode>();
	if (!GM)	return false;
	
	TSubclassOf<ANGPawnBase> UnitClass = GM->GetUnitClass(UnitName);
	
	FGridAddress SpawnGridAddress(EmptyGridIndex.GetValue(), EGridType::Wait, PS);
	
	FVector SpawnLoc = UGridMapHelper::GetWorldLocation(SpawnGridAddress);
	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLoc);

	ANGUnitPawn* NewPawn = SpawnPawn<ANGUnitPawn>(World, UnitClass, SpawnTransform, OwnerController);
	if (!NewPawn)	return false;
	
	//여기서 찾은 그리드에 값 기입
	FGridData GridData;
	GridData.PlacedPawn = NewPawn;
	
	NewPawn->SetPawnOnGrid(SpawnGridAddress);

	Pocket->ControlPocketSpawning(NewPawn);
	
	return true;
}

ANGPawnBase* UNGSpawnHelper::Internal_SpawnPawn(UObject* WorldContextObject, TSubclassOf<ANGPawnBase> PawnClass,
                                                FTransform SpawnTransform,
                                                AActor* Owner)
{
	if (!Owner->HasAuthority())	return nullptr;
	//여기서부터 아래가 소환로직
	
	UNGPoolSubSystem* Pool = WorldContextObject->GetWorld()->GetSubsystem<UNGPoolSubSystem>();
	if (!Pool)	return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	/// 캐릭터 스폰 오프셋이 발끝기준으로 소환되게 조정
	if (const ANGPawnBase* DefaultPawn = PawnClass.GetDefaultObject())
	{
		if (const UCapsuleComponent* Capsule = DefaultPawn->GetCapsuleComponent())
		{
			float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			SpawnTransform.AddToTranslation(FVector(0.f, 0.f, HalfHeight));
		}
	}
	
	ANGPawnBase* NewPawn = Pool->AcquirePawn(PawnClass, SpawnTransform, SpawnParams);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("NewPawn is nullptr"));
		return nullptr;
	}

	if (UCapsuleComponent* Capsule = NewPawn->GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_SelectableUnit, ECR_Block);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
	return NewPawn;
}
