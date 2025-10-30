// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/WidgetController/RollShopWidgetController.h"

#include "Components/NGPocketComponent.h"
#include "Game/NGGameState.h"
#include "Player/NGPlayerController.h"

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
		if (UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
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
			int32 NewLevel = FMath::Clamp(Pocket->PlayerLevel + 1, 1, 5);;
			Pocket->PlayerLevel = NewLevel;
			return NewLevel;
		}
	}
	
	return 1;
}

void URollShopWidgetController::BuyUnitFromPocket(FName UnitName)
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
		{
			Pocket->AddUnitToBuyingPocket(UnitName);
			UE_LOG(LogTemp, Warning, TEXT("BuyUnitFromPocket Success"));
		}
	}
}
