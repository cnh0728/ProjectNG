// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Player/NGPlayerState.h"

#include "Combat/GridMapManager.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGDeveloperSettings.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerController.h"

ANGPlayerState::ANGPlayerState() : PlayerLevel(1)
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
	DOREPLIFETIME(ANGPlayerState, PlayerLevel);
}

void ANGPlayerState::SpawnGridMapManager()
{
	//TODO: 유저별 인덱스 받고 인덱스에 맞는 위치 주입
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
		
	FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	
	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		int32 UserIndex = GS->GetPlayerId(this);
		FVector Margin(GS->GridMargin * (UserIndex / 4), GS->GridMargin * (UserIndex % 4), 0.f);
		SpawnTransform.AddToTranslation(Margin);
	}
	
	TSoftClassPtr<AGridMapManager> GridMapClass = GetDefault<UNGDeveloperSettings>()->GridMapClass;
	if (GridMapClass.IsValid())
	{
		if (UClass* GridMapManagerBPClass = GridMapClass.LoadSynchronous())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = PC;
			GridManager = GetWorld()->SpawnActor<AGridMapManager>(GridMapManagerBPClass, SpawnTransform, SpawnParams);
			
			if (GridManager)
			{
				FGridBuildData BuildData(8, 8, 100.f, 1, 9, 150.f);
				GridManager->Initialize(BuildData, PC->GetPlayerState<ANGPlayerState>());
			}
		}
	}
}

void ANGPlayerState::InitializePostLogin()
{
	SpawnGridMapManager();
	
}

void ANGPlayerState::CaptureSnapShot()
{
	CombatGridMapSnapShot = CombatGridMap;
}

int32 ANGPlayerState::GetUserIndex()
{
	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		return GS->GetPlayerId(this);
	}
	
	return -1;
}

void ANGPlayerState::RestoreInitialGrid()
{
	if (!HasAuthority())	return;
	
	for (int32 i = 0; i < CombatGridMapSnapShot.GridInfo.Num(); i++)
	{
		FGridData& GridData = CombatGridMapSnapShot.GridInfo[i];

		if (GridData.PlacedPawn)
		{
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

