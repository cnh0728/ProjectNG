// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Player/NGPlayerState.h"

#include "Combat/GridMapManager.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGDeveloperSettings.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"
#include "Pawn/NGSpectatorPawn.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerController.h"

ANGPlayerState::ANGPlayerState()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// GAS를 원활히 사용하기 위함
	SetNetUpdateFrequency(100.0f);

	AbilitySystemComponent = CreateDefaultSubobject<UNGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	PlayerPocket = CreateDefaultSubobject<UNGPocketComponent>("PocketComponent");
}

void ANGPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGPlayerState, CombatGridMap);
	DOREPLIFETIME(ANGPlayerState, WaitGridMap);
	DOREPLIFETIME(ANGPlayerState, EnemyWaitGridMap);
	DOREPLIFETIME(ANGPlayerState, PlayerPocket);
	DOREPLIFETIME(ANGPlayerState, GridManager);
}

void ANGPlayerState::SpawnGridMapManager()
{
	//TODO: 유저별 인덱스 받고 인덱스에 맞는 위치 주입
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
		
	FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	
	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		FVector Margin(GS->GridMargin * (UserIndex / 4), GS->GridMargin * (UserIndex % 4), 0.f);
		SpawnTransform.AddToTranslation(Margin);
	}
	
	TSoftClassPtr<AGridMapManager> GridMapClass = GetDefault<UNGDeveloperSettings>()->GridMapClass;
	if (GridMapClass.IsValid())
	{
		if (UClass* GridMapManagerBPClass = GridMapClass.LoadSynchronous())
		{
			FGridBuildData BuildData(8, 8, 100.f, 1, 9, 150.f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = PC;
			GridManager = GetWorld()->SpawnActor<AGridMapManager>(GridMapManagerBPClass, SpawnTransform, SpawnParams);
			if (GridManager)
			{
				GridManager->SetOwnerPS(this);
				GridManager->Initialize(BuildData, UserIndex);
			}
			
			ANGSpectatorPawn* SpectatorPawn = GetPawn<ANGSpectatorPawn>();
			if (SpectatorPawn)
			{
				SpectatorPawn->InitializeGridManager(GridManager);
			}
		}
	}
	
}

void ANGPlayerState::InitializePostLogin(uint32 AssignedIndex)
{
	SetUserIndex(AssignedIndex);
	SpawnGridMapManager();
}

void ANGPlayerState::CaptureSnapShot()
{
	CombatGridMapSnapShot = CombatGridMap;
}

void ANGPlayerState::RestoreInitialGrid()
{
	if (!HasAuthority())	return;
	
	UE_LOG(LogTemp, Warning, TEXT("Restoring Initial Grid State11"));
	
	for (int32 i = 0; i < CombatGridMapSnapShot.GridInfo.Num(); i++)
	{
		FGridData& GridData = CombatGridMapSnapShot.GridInfo[i];

		//이거 snapshot 그리드를 찍지말고 폰들을 찍는게 나을듯?
		if (GridData.PlacedPawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("Restoring Initial Grid State22"));
			FIntVector2 OriginalIndex = CombatGridMapSnapShot.ConvertIndexToPoint(i);

			FGridAddress GridAddress(OriginalIndex, EGridType::Combat, this);
			
			GridData.PlacedPawn->MovePawnOnGrid(GridAddress);
			
			GridData.PlacedPawn->TurnPawnState(EPawnState::Wait);
		}
	}
	
	for (int32 i = 0; i<EnemyWaitGridMap.GridInfo.Num(); i++)
	{
		FIntVector2 Index = EnemyWaitGridMap.ConvertIndexToPoint(i);
		EnemyWaitGridMap.EmptyGridMap(Index);
	}
}

void ANGPlayerState::PrepareStartCombat()
{
	FHexGridMap& GridMap = GetCombatGridMap();
			
	for (FGridData GridData : GridMap.GridInfo)
	{
		ANGPawnBase* PlacedPawn = GridData.PlacedPawn;
		if (IsValid(PlacedPawn))
		{
			PlacedPawn->TurnPawnState(EPawnState::Combat);
		}
	}
}



void ANGPlayerState::SetUserIndex(uint32 Idx)
{
	UserIndex = Idx;
	UE_LOG(LogTemp, Warning, TEXT("Setting User Index: %i, PC: %p"), UserIndex, GetOwner<ANGPlayerController>());
}
