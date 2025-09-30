// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/NGPocketComponent.h"

#include "Core/NGShopProbability.h"
#include "Core/Game/NGGameState.h"
#include "Net/UnrealNetwork.h"


UNGPocketComponent::UNGPocketComponent()
{
	SetIsReplicated(true);
}

void UNGPocketComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNGPocketComponent, RollPocket);
}

void UNGPocketComponent::RequestRoll()
{
	Server_RequestRoll();
}

void UNGPocketComponent::OnRep_RollPocket()
{
	OnUnitsUpdated.Broadcast();
}

void UNGPocketComponent::Server_RequestRoll_Implementation()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (!OwnerController || !ProbabilityTable) return;

	ANGGameState* GameState = GetWorld()->GetGameState<ANGGameState>();
	int32 PlayerLevel = 1; // TODO: 실제 플레이어 레벨을 가져옴

	if (!GameState) return;

	// 기존 포켓 안의 유닛을 다시 반환
	for (TSubclassOf<AActor> UnitToReturn : RollPocket)
	{
		GameState->ReturnUnitToPool(UnitToReturn);
	}
	RollPocket.Empty();

	// 레벨에 맞는 확률 데이터 가져오기
	FString RowName = FString::FromInt(PlayerLevel);
	FShopProbability* ProbabilityData = ProbabilityTable->FindRow<FShopProbability>(*RowName, TEXT(""));
	if (!ProbabilityData) return;

	for (int32 i = 0; i < ShopSlotCount; ++i)
	{
		float RollValue = FMath::FRand(); // 0.0 - 1.0 사이의 랜덤 값

		EUnitTier SelectedTier;
		if (RollValue < ProbabilityData->PercentOfTier1)
		{
			SelectedTier = EUnitTier::Tier1;
		}
		else if (RollValue < ProbabilityData->PercentOfTier1 + ProbabilityData->PercentOfTier2)
		{
			SelectedTier = EUnitTier::Tier2;
		}
		else
		{
			SelectedTier = EUnitTier::Tier3;
		}

		TSubclassOf<AActor> SelectedUnitClass = GameState->GetRandomUnitByTier(SelectedTier);

		if (SelectedUnitClass && GameState->GrabUnitFromPool(SelectedUnitClass))
		{
			RollPocket.Add(SelectedUnitClass);
		}
		else
		{
			i--; // 다시 시도
		}
	}

	OnRep_RollPocket();
}

