// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/HUD/NGHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/NGUserWidget.h"
#include "UI/WidgetController/NGRollShopWidgetController.h"

UNGRollShopWidgetController* ANGHUD::GetRollShopWidgetController(const FWidgetParams& WidgetControllerParams)
{
	if (RollShopWidgetController == nullptr)
	{
		RollShopWidgetController = NewObject<UNGRollShopWidgetController>(this, RollShopWidgetControllerClass);
		RollShopWidgetController->AssignWidgetControllerParams(WidgetControllerParams);
		RollShopWidgetController->BindCallbacksToDependencies();
	}
	
	return RollShopWidgetController;
}

void ANGHUD::InitializeHUD(APlayerController* PC, APlayerState* PS, /*UAbilitySystemComponent* ASC,*/ UAttributeSet* AS)
{
	checkf(MainWidgetClass, TEXT("[HUD] MainWidgetClass not initialized"));
	checkf(RollShopWidgetControllerClass, TEXT("[HUD] RollShopWidgetController not initialized"));
	
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), MainWidgetClass);
	MainWidget = Cast<UNGUserWidget>(Widget);

	const FWidgetParams WidgetParams(PC, PS, AS);
	UNGRollShopWidgetController* MainWidgetC = GetRollShopWidgetController(WidgetParams); // TODO. 테스트용 롤링샵 UI 출력중, 나중에 메인 위젯으로 다시 변경 필요.

	// 위젯 컨트롤러 연결
	MainWidget->ConnectWidgetController(MainWidgetC);
	MainWidgetC->BroadcastInitialValues();

	Widget->AddToViewport();
}
