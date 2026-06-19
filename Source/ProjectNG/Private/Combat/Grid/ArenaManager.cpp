// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/Grid/ArenaManager.h"

#include "Combat/Grid/Arena.h"
#include "Components/NGPocketComponent.h"
#include "Pawn/NGSpectatorPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerState.h"


// Sets default values
AArenaManager::AArenaManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AArenaManager::PossessArena(const FArenaAddress& NewArenaAddress)
{
	// FArenaAddress PreArenaAddress = PossessArenaAddress; 

	PossessArenaAddress = NewArenaAddress;
	
	MigrationUnits(NewArenaAddress);
	
	PossessSpecPawn(NewArenaAddress);
}

void AArenaManager::MigrationUnits(const FArenaAddress& NewAddress) const
{
	if (NewAddress.Arena)
	{
		ANGPlayerState* PocketOwnerPS = GetOwner<ANGPlayerState>();
		ANGPlayerState* NewPS = NewAddress.Arena->GetOwnerPS();
		
		if (PocketOwnerPS)
		{
			if (UNGPocketComponent* PocketComponent = PocketOwnerPS->GetPlayerPocket())
			{
				for (ANGPawnBase* Unit : PocketComponent->GetOwnedUnitPocket())
				{
					if (!IsValid(Unit))	continue;
					
					FGridAddress GridAddress = Unit->GetGridAddress();
					FIntVector2 MirroredIdx = UGridMapHelper::GetMirroredIndex(*UGridMapHelper::GetGridMap(GridAddress), GridAddress.GridIndex);
					
					FGridAddress NewGridAddress(MirroredIdx, EGridType::Combat, NewPS, GridAddress.DirtyFlag);
					// 돌아올때는 그리드 리셋을 시키기 때문에 Wait만 
					
					if (NewAddress.PossessArenaIdentification == EPossessArenaIdentification::Home && GridAddress.GridType == EGridType::Combat)
					{
						//전투 종료시 (Home으로 돌아가는 Combat) CombatGrid는 Snapshot으로 리셋이기 때문에 배치하지 않음.
						continue;
					}
					
					if (GridAddress.GridType == EGridType::EnemyWait)
					{
						NewGridAddress.GridType = EGridType::Wait;
					}
					else if (GridAddress.GridType == EGridType::Wait)
					{
						NewGridAddress.GridType = EGridType::EnemyWait;
					}
					Unit->SetPawnOnGrid(NewGridAddress);
				}
			}
		}
		
	}
}

void AArenaManager::PossessSpecPawn(const FArenaAddress& ArenaAddress) const
{	
	if (ANGPlayerState* PS = GetOwner<ANGPlayerState>())
	{
		if (ANGSpectatorPawn* SpecPawn = PS->GetPawn<ANGSpectatorPawn>())
		{
			//이떄 아레나 Camera들 Transform이 안만들어져있음
			FTransform CameraTransform = 
				ArenaAddress.PossessArenaIdentification == EPossessArenaIdentification::Home 
			? ArenaAddress.Arena->GetHomeCameraTransform() 
			: ArenaAddress.Arena->GetAwayCameraTransform();
			
			
			//여기는 지금 서버에서 들어와서 Client로 RPC해야함
			SpecPawn->Client_PossessCamera(CameraTransform);
		}
	}
}

// Called when the game starts or when spawned
void AArenaManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AArenaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

