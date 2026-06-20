// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/NGShopControlWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/NGPlayerController.h"

void UNGShopControlWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (FleeButton)
	{
		FleeButton->OnClicked.AddDynamic(this, &UNGShopControlWidget::OnFleeButtonClicked);
	}
}

void UNGShopControlWidget::OnFleeButtonClicked()
{
	if (WidgetController)
	{
		if (ANGPlayerController* PC = GetOwningPlayer<ANGPlayerController>())
		{
			ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>();
			if (PS ? PS->GetGameState() == EGameState::Combat : false)
			{
				PC->Server_RequestFlee();
			}
		}
	}
}

void UNGShopControlWidget::UpdateGoldText(float NewGold)
{
	if (GoldText)
	{
		FText GoldNumberText = FText::AsNumber(FMath::FloorToInt(NewGold));

		FText FinalGoldText = FText::Format(NSLOCTEXT("UI", "GoldFormat", "Gold: {0}"), GoldNumberText);
        
		GoldText->SetText(FinalGoldText);
	}
}
