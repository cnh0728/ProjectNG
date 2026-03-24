// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/WidgetController/RollShopWidgetController.h"

#include "Character/NGUnitCharacter.h"
#include "Combat/CombatManager.h"
#include "Combat/GridMapManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/NGPocketComponent.h"
#include "Core/NGUnitData.h"
#include "Game/NGGameState.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"

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
	if (ANGGameState* GS = GetWorld()->GetGameState<ANGGameState>())
	{
		if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
		{
			if (UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
			{
				if (ACombatManager* CombatManager = GS->GetCombatManager())
				{
					if (CombatManager->SpawnUnitCharacter(UnitName))
					{
						Pocket->AddUnitToBuyingPocket(UnitName);
						UE_LOG(LogTemp, Display, TEXT("BuyUnitFromPocket Success"));
					}
				}
			}
		}
	}
}
