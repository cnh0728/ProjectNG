// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/WidgetController/RollShopWidgetController.h"

#include "Combat/GridMapManager.h"
#include "Components/NGPocketComponent.h"
#include "GameModes/NGInGameGameMode.h"
#include "Player/NGPlayerController.h"

class ANGGameState;

void URollShopWidgetController::BroadcastInitialValues()
{
	
}

void URollShopWidgetController::BindCallbacksToDependencies()
{
	
}

void URollShopWidgetController::GetPlayerRollPocket(TArray<FName>& RollPockets) const
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (const UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
		{
			RollPockets = Pocket->GetRollPocket();
		}
	}
}

void URollShopWidgetController::RequestPlayerRollPocket() const
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
		{
			UE_LOG(LogTemp, Warning, TEXT("Pocket Address: %p, Owner: %s"), Pocket, *Pocket->GetOwner()->GetName());
			Pocket->RequestRoll();
		}
	}
}

int32 URollShopWidgetController::GainPlayerLevel() const
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
		{
			int32 NewLevel = FMath::Clamp(Pocket->PlayerLevel + 1, 1, 5);
			Pocket->PlayerLevel = NewLevel;
			return NewLevel;
		}
	}
	
	return 1;
}

void URollShopWidgetController::BuyUnitFromPocket(FName UnitName)
{
	//여기서 그리드에 칸이 비어있는지 체크 후 사야함
	if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>())
	{
		if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
		{
			if (UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
			{
				if (AGridMapManager* GridManager = GM->GetGridMapManager())
				{
					if (GridManager->SpawnUnitCharacter(UnitName))
					{
						Pocket->AddUnitToBuyingPocket(UnitName);
						UE_LOG(LogTemp, Display, TEXT("BuyUnitFromPocket Success"));
					}
				}
			}
		}
	}
}
