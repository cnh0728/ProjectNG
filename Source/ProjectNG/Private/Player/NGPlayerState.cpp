// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Player/NGPlayerState.h"

#include "Combat/Grid/Arena.h"
#include "Combat/Grid/ArenaManager.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGDeveloperSettings.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"
#include "Pawn/NGUnitPawn.h"
#include "Player/NGPlayerController.h"

ANGPlayerState::ANGPlayerState() : PlayerLevel(1), CurrentState(EGameState::Maintaining)
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
	DOREPLIFETIME(ANGPlayerState, HomeArena);
	DOREPLIFETIME(ANGPlayerState, PlayerLevel);
	DOREPLIFETIME(ANGPlayerState, CurrentState);
}

void ANGPlayerState::SpawnGridMapManager()
{
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
		
	FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	
	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		int32 UserIndex = GS->GetPlayerId(this);
		FVector Margin(GS->GridMargin * (UserIndex / 4), GS->GridMargin * (UserIndex % 4), 0.f);
		SpawnTransform.AddToTranslation(Margin);
	}
	
	TSoftClassPtr<AArena> GridMapClass = GetDefault<UNGDeveloperSettings>()->ArenaClass;
	if (GridMapClass.IsValid())
	{
		//SM들을 배정해둬서 BP로 가져와야함
		if (UClass* GridMapManagerBPClass = GridMapClass.LoadSynchronous())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = PC;
			HomeArena = GetWorld()->SpawnActor<AArena>(GridMapManagerBPClass, SpawnTransform, SpawnParams);
			
			if (HomeArena)
			{
				FGridBuildData BuildData(8, 8, 100.f, 1, 9, 150.f);
				HomeArena->Initialize(BuildData, PC->GetPlayerState<ANGPlayerState>());
			}
			
			FActorSpawnParameters ArenaSpawnParams;
			ArenaSpawnParams.Owner = PC ? PC->GetPlayerState<ANGPlayerState>() : nullptr;
			
			ArenaManager = GetWorld()->SpawnActor<AArenaManager>(AArenaManager::StaticClass(), SpawnTransform, ArenaSpawnParams);
			
			if (ArenaManager)
			{
				FArenaAddress ArenaAddress(HomeArena, EPossessArenaIdentification::Home);
				ArenaManager->PossessArena(ArenaAddress);
			}
		}
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("NGDeveloperSettings - Invalid ArenaClass!"));
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
			
			GridData.PlacedPawn->TranslatePawnOnGrid(GridAddress);
			
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

