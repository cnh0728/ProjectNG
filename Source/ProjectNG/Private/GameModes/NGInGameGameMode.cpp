// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "GameModes/NGInGameGameMode.h"

#include "Combat/CombatManager.h"


void ANGInGameGameMode::RequestStartCombat(APlayerController* PC)
{
	if (CurrentState == EGameState::Combat)	return;
	
	if (!ActiveCombatManager)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		ActiveCombatManager = GetWorld()->SpawnActor<ACombatManager>(CombatManagerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
	
	//ActiveCombatManager가 없을수 없지만 혹시모르니
	if (ActiveCombatManager)
	{
		CurrentState = EGameState::Combat;
		
		//이거는 따로 웨이브 관리하는거 만들어서 그때 값 가져와서 넣어야할듯?
		FCombatSettingData SettingData;
		SettingData.EnemyCount = 3;
		
		ActiveCombatManager->StartCombat(SettingData, PC);
	}
}

void ANGInGameGameMode::OnCombatFinished(const FCombatResultData& ResultData)
{
	if (CurrentState != EGameState::Combat)	return;
	
	UE_LOG(LogTemp, Log, TEXT("Combat Finished"));
	
	//Combat은 모험중이 아니면 들어갈 수 없음
	CurrentState = EGameState::Exploration;
	
	//ResultData로 점수나 그런거 반영하기

}

void ANGInGameGameMode::ReportPawnDeath(ANGPawnBase* DeadPawn)
{
	if (ActiveCombatManager)
	{
		ActiveCombatManager->PawnDied(DeadPawn);
	}
}

void ANGInGameGameMode::ChangeState(const EGameState NewState)
{
	CurrentState = NewState;
}
