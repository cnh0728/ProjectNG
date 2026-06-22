// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/WidgetController/NGRollShopWidgetController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NGPlayerAttributeSet.h"
#include "Components/NGPocketComponent.h"
#include "Player/NGPlayerController.h"

class ANGGameState;

void UNGRollShopWidgetController::BroadcastInitialValues()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
	{
		if (const UNGPlayerAttributeSet* Asset = ASC->GetSet<UNGPlayerAttributeSet>())
		{
			OnGoldChanged.Broadcast(Asset->GetGold());
		}
	}
}

void UNGRollShopWidgetController::BindCallbacksToDependencies()
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerState))
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UNGPlayerAttributeSet::GetGoldAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				UE_LOG(LogTemp, Log, TEXT("New Gold Value: %f"), Data.NewValue);
				OnGoldChanged.Broadcast(Data.NewValue);
			}
		);
	}
}

void UNGRollShopWidgetController::GetPlayerRollPocket(TArray<FName>& RollPockets) const
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
			if (UNGAbilitySystemComponent* ASC = PS->GetNGAbilitySystemComponent())
			{
				if (const UNGPlayerAttributeSet* AttributeSet = ASC->GetSet<UNGPlayerAttributeSet>())
				{
					int32 CurrentLevel = FMath::RoundToInt(AttributeSet->GetLevel());
					int32 NewLevel = FMath::Clamp(CurrentLevel + 1, 1, 5);
					ASC->SetNumericAttributeBase(UNGPlayerAttributeSet::GetLevelAttribute(), static_cast<float>(NewLevel));
        
					return NewLevel;
				}
			}
		}
	}
	
	return 1;
}

void UNGRollShopWidgetController::BuyUnitFromPocket(FName UnitName)
{
	if (ANGPlayerController* NGP = Cast<ANGPlayerController>(PlayerController))
	{
		if (NGP->IsLocalController())
		{
			NGP->Server_RequestBuyUnit(UnitName);
		}
	}
}
