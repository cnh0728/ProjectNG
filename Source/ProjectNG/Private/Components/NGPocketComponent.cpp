// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/NGPocketComponent.h"

#include "Core/NGShopProbability.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"


UNGPocketComponent::UNGPocketComponent()
{
	SetIsReplicatedByDefault(true);
}

void UNGPocketComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNGPocketComponent, RollPocket, COND_OwnerOnly); //요청보낸 주인한테만 동기화
}

void UNGPocketComponent::RequestRoll()
{
	AActor* MyOwner = GetOwner();
	FString RoleStr = MyOwner->HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]");
    
	// 1. 내 주인(PC)이 서버와 연결된 소켓(NetConnection)을 가지고 있는지 확인!
	bool bHasConnection = (MyOwner->GetNetConnection() != nullptr);
    
	UE_LOG(LogTemp, Warning, TEXT("%s [RequestRoll] Addr: %p | Has NetConnection: %s"), 
		*RoleStr, this, bHasConnection ? TEXT("TRUE") : TEXT("FALSE"));

	if (!bHasConnection && !MyOwner->HasAuthority())
	{
		UE_LOG(LogTemp, Error, TEXT("!!! ERROR !!! 당신은 지금 네트워크 연결이 없는 가짜 객체에서 RPC를 시도했습니다. 패킷이 증발합니다!"));
		return;
	}

	Server_RequestRoll();
}

void UNGPocketComponent::AddUnitToBuyingPocket(FName UnitName)
{
	RollPocket.Remove(UnitName);
}

void UNGPocketComponent::OnRep_RollPocket()
{
	if (ANGPlayerController* PC = Cast<ANGPlayerController>(GetOwner()))
	{
		FString RoleStr = GetOwner()->HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]");
		auto Owner = GetOwner();
		
		UE_LOG(LogTemp, Warning, TEXT("[OnRep] Pocket Addr: %p, Owner Addr: %p"), this, Owner);
		
		// 2. 델리게이트 바인딩 상태 확인 (중요)
		if (PC->OnUnitsUpdated.IsBound())
		{
			UE_LOG(LogTemp, Warning, TEXT("OnRep_RollChange: Delegate is Bound. Broadcaster Addr: %p"), &(PC->OnUnitsUpdated));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("OnRep_RollChange: NO ONE is listening to this delegate!"));
		}
		PC->OnUnitsUpdated.Broadcast();
		
	}
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
	
	OnRep_RollPocket();
}
