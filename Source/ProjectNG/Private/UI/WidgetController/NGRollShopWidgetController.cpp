// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/WidgetController/NGRollShopWidgetController.h"

#include "Components/NGPocketComponent.h"
#include "Player/NGPlayerController.h"

class ANGGameState;

void UNGRollShopWidgetController::BroadcastInitialValues()
{
	
}

void UNGRollShopWidgetController::BindCallbacksToDependencies()
{
	
}

void UNGRollShopWidgetController::GetPlayerRollPocket(TArray<FGameplayTag>& RollPockets) const
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (const UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
		{
			RollPockets = Pocket->GetRollPocket();
		}
	}
}

void UNGRollShopWidgetController::RequestPlayerRollPocket() const
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (UNGPocketComponent* Pocket = NGP->GetPlayerPocket())
		{
			Pocket->RequestRoll();
		}
	}
}

int32 UNGRollShopWidgetController::GainPlayerLevel() const
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (ANGPlayerState* PS = NGP->GetPlayerState<ANGPlayerState>())
		{
			int32 NewLevel = FMath::Clamp(PS->GetPlayerLevel() + 1, 1, 5);
			PS->SetPlayerLevel(NewLevel);
			return NewLevel;
		}
	}
	
	return 1;
}

void UNGRollShopWidgetController::BuyUnitFromPocket(FGameplayTag UnitTag)
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (NGP->IsLocalController())
		{
			NGP->Server_RequestBuyUnit(UnitTag);
		}
	}
}
