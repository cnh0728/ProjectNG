// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGSpawnHelper.h"

#include "Components/CapsuleComponent.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGPoolSubSystem.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameMode.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"

bool UNGSpawnHelper::SpawnUnitPawn(ANGPlayerController* OwnerController, FName UnitName)
{
	if (!OwnerController) return false;
	
	ANGPlayerState* PS = OwnerController->GetPlayerState<ANGPlayerState>();
	if (!PS) return false;
	
	FQuadGridMap& WaitGridMap = PS->GetWaitGridMap();
	
	TOptional<FIntVector2> EmptyGridIndex = WaitGridMap.GetEmptyGridIndex();
	
	if (!EmptyGridIndex.IsSet()) return false;

	FGridAddress SpawnGridAddress(EmptyGridIndex.GetValue(), EGridType::Wait, PS, 0);

	return SpawnUnitPawnAtGrid(OwnerController, UnitName, SpawnGridAddress);
}

bool UNGSpawnHelper::SpawnUnitPawnAtGrid(ANGPlayerController* OwnerController, FName UnitName, const FGridAddress& SpawnGridAddress)
{
	if (!OwnerController || !OwnerController->HasAuthority()) return false;

	ANGPlayerState* PS = OwnerController->GetPlayerState<ANGPlayerState>();
	if (!PS || SpawnGridAddress.GridOwnerPS != PS) return false;

	FGridMapBase* GridMap = UGridMapHelper::GetGridMap(SpawnGridAddress);
	if (!GridMap || !GridMap->IsValidIndex(SpawnGridAddress.GridIndex) || !GridMap->IsGridIndexEmpty(SpawnGridAddress.GridIndex))
	{
		return false;
	}

	UNGPocketComponent* Pocket = PS->GetPlayerPocket();
	if (!Pocket) return false;

	UWorld* World = OwnerController->GetWorld();
	if (!World)	return false;

	ANGInGameMode* GM = World->GetAuthGameMode<ANGInGameMode>();
	if (!GM) return false;

	const FUnitData* UnitData = GM->GetUnitData(UnitName);
	if (!UnitData || !UnitData->IdentificationTag.IsValid()) return false;

	TSubclassOf<ANGPawnBase> UnitClass = UnitData->UnitClass;
	if (!UnitClass) return false;

	FVector SpawnLoc = UGridMapHelper::GetWorldLocation(SpawnGridAddress);
	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLoc);

	ANGUnitPawn* NewPawn = SpawnPawn<ANGUnitPawn>(World, UnitClass, SpawnTransform, OwnerController);
	if (!NewPawn) return false;
	
	if (UCapsuleComponent* Capsule = NewPawn->GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_SelectableUnit, ECR_Block);
	}
	
	NewPawn->Initialize(PS);
	NewPawn->SetIdentificationTag(UnitData->IdentificationTag);
	NewPawn->Activate();
	
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
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
	return NewPawn;
}
