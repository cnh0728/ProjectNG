// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/NGPocketComponent.h"

#include "Core/NGShopProbability.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"


UNGPocketComponent::UNGPocketComponent()
{
	//SetIsReplicatedByDefault(true);
}

void UNGPocketComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNGPocketComponent, RollChangeVersion);
}

void UNGPocketComponent::RequestRoll()
{
	Server_RequestRoll();
}

void UNGPocketComponent::AddUnitToBuyingPocket(FName UnitName)
{
	RollPocket.Remove(UnitName);
}

void UNGPocketComponent::OnRep_RollChange()
{
	UE_LOG(LogTemp, Error, TEXT("[C++] OnRep Instance: %s"), *this->GetPathName());
	
	UE_LOG(LogTemp, Error, TEXT("[OnRep] Pocket Addr: %p, Owner Addr: %p"), this, GetOwner());
	
	// 2. 델리게이트 바인딩 상태 확인 (중요)
	if (OnUnitsUpdated.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_RollChange: Delegate is Bound. Broadcaster Addr: %p"), &OnUnitsUpdated);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OnRep_RollChange: NO ONE is listening to this delegate!"));
	}
	
	OnUnitsUpdated.Broadcast();
}

void UNGPocketComponent::Server_RequestRoll_Implementation()
{
	if (!GetOwner()->HasAuthority())	return;
	
	checkf(ProbabilityTable, TEXT("[PocketComponent] Not initialized Probability table."))
	
	AController* OwnerController = Cast<AController>(GetOwner());
	if (!OwnerController || !ProbabilityTable) return;

	ANGGameState* GameState = GetWorld()->GetGameState<ANGGameState>();
	if (!GameState) return;
	
	// 레벨에 맞는 확률 데이터 가져오기
	FString RowName = FString::FromInt(PlayerLevel);
	FShopProbability* ProbabilityData = ProbabilityTable->FindRow<FShopProbability>(*RowName, TEXT(""));
	
	if (!ProbabilityData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProbabilityData is not found, PlayerLevel has invalid value."));
		return;
	}
	
	// 기존 포켓 안의 유닛을 다시 반환
	for (FName UnitToReturn : RollPocket)
	{
		GameState->ReturnUnitToPool(UnitToReturn, 1);
	}
	RollPocket.Empty();

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

		FName SelectedUnitRowName = GameState->GetRandomUnitByTier(SelectedTier);

		if (!SelectedUnitRowName.IsNone() && GameState->IsExistUnit(SelectedUnitRowName))
		{
			GameState->GrabUnitFromPool(SelectedUnitRowName);
			RollPocket.Add(SelectedUnitRowName);
		}
		else
		{
			i--; // 다시 시도
		}
	}
	
	RollChangeVersion++;
	OnRep_RollChange(); //서버 본인도 호출해야하기 때문에
}
