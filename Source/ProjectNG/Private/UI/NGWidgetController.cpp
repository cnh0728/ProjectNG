// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/NGWidgetController.h"

void UNGWidgetController::AssignWidgetControllerParams(const FWidgetParams& InWidgetParams)
{
	PlayerController = InWidgetParams.PlayerController;
	PlayerState = InWidgetParams.PlayerState;
	AbilitySystemComponent = InWidgetParams.AbilitySystemComponent;
	AttributeSet = InWidgetParams.AttributeSet;
}
