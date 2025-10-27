// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/HUD/NGHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/NGUserWidget.h"
#include "UI/WidgetController/MainWidgetController.h"

UMainWidgetController* ANGHUD::GetMainWidgetController(const FWidgetParams& WidgetControllerParams)
{
	if (MainWidgetController == nullptr)
	{
		MainWidgetController = NewObject<UMainWidgetController>(this, MainWidgetControllerClass);
		MainWidgetController->AssignWidgetControllerParams(WidgetControllerParams);
		MainWidgetController->BindCallbacksToDependencies();
	}
	
	return MainWidgetController;
}

void ANGHUD::InitializeHUD(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(MainWidgetClass, TEXT("[HUD] MainWidget not initialized"));
	checkf(MainWidgetControllerClass, TEXT("[HUD] MainWidgetController not initialized"));

	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), MainWidgetClass);
	MainWidget = Cast<UNGUserWidget>(Widget);

	const FWidgetParams WidgetParams(PC, PS, ASC, AS);
	UMainWidgetController* MainWidgetC = GetMainWidgetController(WidgetParams);

	// 위젯 컨트롤러 연결
	MainWidget->ConnectWidgetController(MainWidgetC);
	MainWidgetC->BroadcastInitialValues();

	Widget->AddToViewport();
}
