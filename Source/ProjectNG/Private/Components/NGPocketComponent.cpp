// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Components/NGPocketComponent.h"

#include "Core/NGPoolSubSystem.h"
#include "Core/NGShopProbability.h"
#include "Core/NGSpawnHelper.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Game/NGUnitDataManager.h"
#include "GameModes/NGInGameGameMode.h"
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
}

void UNGPocketComponent::CheckAndMergeUnit(FGameplayTag IdentificationTag)
{
	
	// 서버가 아니면 리턴
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	UNGUnitDataManager* DataManager = GetWorld()->GetGameInstance()->GetSubsystem<UNGUnitDataManager>();
	if (!DataManager) return;
	
	
	TArray<ANGUnitPawn*> SameUnits;
	for (ANGUnitPawn* Unit : OwnedUnitPocket)
	{
		if (Unit && Unit->GetIdentificationTag() == IdentificationTag)
		{
			SameUnits.Add(Unit);
		}
	}
	
	if (SameUnits.Num() >= 3)
	{
		ANGUnitPawn* Mat1 = SameUnits[0]; // 조합의 중심이 될 유닛
		ANGUnitPawn* Mat2 = SameUnits[1];
		ANGUnitPawn* Mat3 = SameUnits[2];
		
		
		const FUnitData* UnitData = DataManager->GetUnitData(IdentificationTag);
		if (!UnitData || !UnitData->NextTierTag.IsValid()) return;
		
		FName NextUnitRowName = DataManager->GetUnitName(UnitData->NextTierTag);
		if (NextUnitRowName.IsNone()) return;
		
		FVector MergeLocation = Mat1->GetActorLocation(); // 첫번째 유닛 위치

		if (UNGPoolSubSystem* Pool = GetWorld()->GetSubsystem<UNGPoolSubSystem>())
		{
			if (ANGPlayerState* PS = GetOwner<ANGPlayerState>())
			{
				FHexGridMap& CombatGridMap = PS->GetCombatGridMap();
				
				CombatGridMap.EmptyGridMap(Mat1->GetPlacedGridIndex());
				CombatGridMap.EmptyGridMap(Mat2->GetPlacedGridIndex());
				CombatGridMap.EmptyGridMap(Mat3->GetPlacedGridIndex());
			}
			
			ControlPocketSelling(Mat1);
			ControlPocketSelling(Mat2);
			ControlPocketSelling(Mat3);
			
			Pool->ReleaseSegment(Mat1);
			Pool->ReleaseSegment(Mat2);
			Pool->ReleaseSegment(Mat3);
		}
		
		// 상위 등급 생성
		if (ANGPlayerState* PS = GetOwner<ANGPlayerState>())
		{
			if (ANGPlayerController* PC = Cast<ANGPlayerController>(PS->GetPlayerController()))
			{
				// 이 스폰이 성공하면 다시 ControlPocketSpawning이 호출되며 연쇄 작용 일어남
				bool bSpawned = UNGSpawnHelper::SpawnUnitPawn(PC, NextUnitRowName);
				if (bSpawned)
				{
					// 클라이언트 RPC 연출 호출
					const FUnitData* NextData = DataManager->GetUnitData(UnitData->NextTierTag);
					if (NextData) Multicast_PlayMergeEffect(MergeLocation, NextData->Tier);
				}
			}
		}
	}
}

void UNGPocketComponent::Multicast_PlayMergeEffect_Implementation(FVector EffectLocation, EUnitTier UnitTier)
{
	// 이 함수는 서버에서 호출되더라도 모든 클라이언트(플레이어의 화면)에서 실행됩니다.
	// 이 안에서 무거운 이펙트 생성, 파티클 폭발, 티어별 사운드 재생 등을 수행합니다.
}

void UNGPocketComponent::ControlPocketSpawning(ANGUnitPawn* NewPawn)
{
	OwnedUnitPocket.AddUnique(NewPawn);
	WaitUnitPocket.AddUnique(NewPawn);
	
	if (NewPawn)
	{
		CheckAndMergeUnit(NewPawn->GetIdentificationTag());
	}
}

void UNGPocketComponent::ControlPocketPlacing(ANGUnitPawn* NewPawn)
{
	PlacedUnitPocket.AddUnique(NewPawn);
	WaitUnitPocket.Remove(NewPawn);
}

void UNGPocketComponent::ControlPocketUnPlacing(ANGUnitPawn* NewPawn)
{
	PlacedUnitPocket.Remove(NewPawn);
	WaitUnitPocket.AddUnique(NewPawn);
}

void UNGPocketComponent::ControlPocketSelling(ANGUnitPawn* NewPawn)
{
	OwnedUnitPocket.Remove(NewPawn);
	PlacedUnitPocket.Remove(NewPawn);
	WaitUnitPocket.Remove(NewPawn);
}

void UNGPocketComponent::Server_RequestRoll_Implementation()
{
	if (!GetOwner()->HasAuthority())	return;
	
	checkf(ProbabilityTable, TEXT("[PocketComponent] Not initialized Probability table."))
	
	if (!ProbabilityTable) return;

	ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>();
	if (!GM) return;
	
	// 레벨에 맞는 확률 데이터 가져오기
	FString RowName = FString::FromInt(PlayerLevel);
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
