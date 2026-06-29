// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Game/NGGameState.h"

#include "Components/NGCombatManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerState.h"

ANGGameState::ANGGameState() : GridMargin(3000.f)
{
}

void ANGGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ANGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGGameState, CurrentPhase);
	DOREPLIFETIME(ANGGameState, CurrentTurn);
	DOREPLIFETIME(ANGGameState, RemainingTime);
	DOREPLIFETIME(ANGGameState, MapNodes);
}

void ANGGameState::SetMapNodes(const TArray<FMapNodeData>& InNodes)
{
	if (HasAuthority())
	{
		MapNodes = InNodes;
		OnRep_MapNodes(); // 서버(로컬)에서도 이벤트 발생을 위해 수동 호출
	}
}

void ANGGameState::OnRep_MapNodes()
{
	OnMapDataReady.Broadcast();
}

uint8 ANGGameState::GetPlayerId(const ANGPlayerState* InPS)
{
	return InPS->GetPlayerId();
}