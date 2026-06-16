// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGBlueprintLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "Player/NGPlayerState.h"
#include "UI/NGWidgetController.h"
#include "UI/HUD/NGHUD.h"

ANGHUD* UNGBlueprintLibrary::GetNGHUD(const UObject* WorldContextObject)
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (PC)
	{
		return Cast<ANGHUD>(PC->GetHUD());
	}

	return nullptr;
}

bool UNGBlueprintLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetParams& OutWidgetParams,
	ANGHUD*& OutHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutHUD = Cast<ANGHUD>(PC->GetHUD());
		if (OutHUD)
		{
			ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>();
			OutWidgetParams.PlayerState = PS;
			OutWidgetParams.PlayerController = PC;
			return true;
		}
	}
	return false;
}

UUnitDetailsWidgetController* UNGBlueprintLibrary::GetUnitDetailsWidgetController(const UObject* WorldContextObject)
{
	FWidgetParams WParams;
	ANGHUD* NGHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WParams, NGHUD))
	{
		// return NGHUD->GetUnitDetailsWidgetController(WParams);
	}
	return nullptr;
}
