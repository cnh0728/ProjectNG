// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Player/NGPlayerState.h"

#include "Combat/GridMapManager.h"
#include "Combat/Weapon/NGProjectile.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGDeveloperSettings.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"

ANGPlayerState::ANGPlayerState()
{
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
	
	if (UClass* GridMapManagerBPClass = GetDefault<UNGDeveloperSettings>()->GridmapClass[AGridMapManager::StaticClass()].LoadSynchronous())
	{
		GridManager = GetWorld()->SpawnActorDeferred<AGridMapManager>(GridMapManagerBPClass, SpawnTransform, PC);
		if (GridManager)
		{
			FGridBuildData BuildData(8, 8, 100.f);
			
			GridManager->Initialize(PC, BuildData);
			GridManager->FinishSpawning(SpawnTransform);
		}
		
		CombatGridMap.Pivot = GridManager->GetActorLocation();
	}
	
}

void ANGPlayerState::InitializeLogin(uint32 AssignedIndex)
{
	SetUserIndex(AssignedIndex);
	SpawnGridMapManager();
}

void ANGPlayerState::SetUserIndex(uint32 Idx)
{
	UserIndex = Idx;
	UE_LOG(LogTemp, Warning, TEXT("Setting User Index: %i, PC: %p"), UserIndex, GetOwner<ANGPlayerController>());
}
