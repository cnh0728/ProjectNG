// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/HUD/LobbyHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/NGUserWidget.h"
#include "UI/WidgetController/LobbyWidgetController.h"

ULobbyWidgetController* ALobbyHUD::GetMainWidgetController(const FWidgetParams& WidgetControllerParams)
{
	if (MainWidgetController == nullptr)
	{
		MainWidgetController = NewObject<ULobbyWidgetController>(this, MainWidgetControllerClass);
		MainWidgetController->AssignWidgetControllerParams(WidgetControllerParams);
		MainWidgetController->BindCallbacksToDependencies();
	}
	
	return MainWidgetController;
}

void ALobbyHUD::InitializeHUD(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(MainWidgetClass, TEXT("[HUD] MainWidgetClass not initialized"));
	checkf(MainWidgetControllerClass, TEXT("[HUD] LobbyWidgetControllerClass not initialized"));
	
	MainWidget = CreateWidget<UNGUserWidget>(GetWorld(), MainWidgetClass);
	
	const FWidgetParams WidgetParams(PC, PS, ASC, AS);
	ULobbyWidgetController* LobbyWidgetController = GetMainWidgetController(WidgetParams);
	
	// 위젯 컨트롤러 연결
	MainWidget->ConnectWidgetController(LobbyWidgetController);
	LobbyWidgetController->BroadcastInitialValues();
	
	MainWidget->AddToViewport();
}
