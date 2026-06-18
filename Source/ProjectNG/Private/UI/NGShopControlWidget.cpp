// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/NGShopControlWidget.h"
#include "Components/Button.h"
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

void UNGShopControlWidget::OnGoldChanged(float NewGold)
{
}
