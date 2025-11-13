// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/NGUserWidget.h"

void UNGUserWidget::ConnectWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	BindWidgetController();
}
