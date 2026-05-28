// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/NGPocketComponent.h"

#include "Core/NGShopProbability.h"
#include "Core/NGUnitData.h"
#include "GameModes/NGInGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"


UNGPocketComponent::UNGPocketComponent()
{
	SetIsReplicatedByDefault(true);
}

void UNGPocketComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UNGPocketComponent, RollShopPocket, COND_OwnerOnly); //요청보낸 주인한테만 동기화
	DOREPLIFETIME_CONDITION(UNGPocketComponent, LastShopAction, COND_OwnerOnly); 
	DOREPLIFETIME(UNGPocketComponent, OwnedUnitPocket);
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
	LastShopAction = EShopActionType::Buy;
	RollShopPocket.Remove(UnitName);
}

void UNGPocketComponent::OnRep_RollPocket()
{
	switch (LastShopAction)
	{
		case EShopActionType::Buy:
		
			break;
		case EShopActionType::Roll:
			UpdateRollUnit();
			break;
		default:
			break;
	}
}

void UNGPocketComponent::UpdateRollUnit()
{
	if (ANGPlayerState* PS = GetOwner<ANGPlayerState>())
	{
		if (ANGPlayerController* PC = Cast<ANGPlayerController>(PS->GetPlayerController()))
		{
			FString RoleStr = GetOwner()->HasAuthority() ? TEXT("[SERVER]") : TEXT("[CLIENT]");
			auto Owner = GetOwner();
			
			UE_LOG(LogTemp, Warning, TEXT("[OnRep] Pocket Addr: %p, Owner Addr: %p"), this, Owner);
			
			// 델리게이트 바인딩 상태 확인
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
}

void UNGPocketComponent::GetPlacedUnits(TArray<ANGPawnBase*>& OutUnits)
{
	for (ANGPawnBase* Unit : OwnedUnitPocket)
	{
		if (Unit)
		{
			if (Unit->GetGridAddress().GridType == EGridType::Combat)
			{
				OutUnits.Add(Unit);
			}
		}
	}
}

void UNGPocketComponent::ControlPocketSpawning(ANGPawnBase* NewPawn)
{
	AddUnitFromPocket(NewPawn);
}

void UNGPocketComponent::ControlPocketSelling(ANGPawnBase* NewPawn)
{
	RemoveUnitFromPocket(NewPawn);
}

void UNGPocketComponent::AddUnitFromPocket(ANGPawnBase* NewPawn)
{
	OwnedUnitPocket.AddUnique(NewPawn);
}

void UNGPocketComponent::RemoveUnitFromPocket(ANGPawnBase* Unit)
{
	OwnedUnitPocket.Remove(Unit);
}

void UNGPocketComponent::Server_RequestRoll_Implementation()
{
	if (!GetOwner()->HasAuthority())	return;
	
	checkf(ProbabilityTable, TEXT("[PocketComponent] Not initialized Probability table."))
	
	if (!ProbabilityTable) return;

	ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>();
	if (!GM) return;
	
	ANGPlayerState* PS = GetOwner<ANGPlayerState>();
	if (!PS) return;
	
	// 레벨에 맞는 확률 데이터 가져오기
	FString RowName = FString::FromInt(PS->GetPlayerLevel());
	FShopProbability* ProbabilityData = ProbabilityTable->FindRow<FShopProbability>(*RowName, TEXT(""));
	
	if (!ProbabilityData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProbabilityData is not found, PlayerLevel has invalid value."));
		return;
	}
	
	// 기존 포켓 안의 유닛을 다시 반환
	for (FName UnitToReturn : RollShopPocket)
	{
		GM->ReturnUnitToPool(UnitToReturn, 1);
	}
	RollShopPocket.Empty();
	
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

		FName SelectedUnitRowName = GM->GetRandomUnitByTier(SelectedTier);

		if (GM->IsExistUnitDataTable())
		{
			if (!SelectedUnitRowName.IsNone() && GM->IsExistUnit(SelectedUnitRowName))
			{
				GM->GrabUnitFromPool(SelectedUnitRowName);
				RollShopPocket.Add(SelectedUnitRowName);
			}
			else
			{
				
				i--; // 다시 시도
			}
		}else
		{
			UE_LOG(LogTemp, Error, TEXT("DataTabel isn't Exist!"));
		}
	}

	LastShopAction = EShopActionType::Roll;
	
	// 서버에서는 롤포켓할필요 없지않나
	// OnRep_RollPocket();
}
