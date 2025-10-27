// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/NGWidgetController.h"
#include "MainWidgetController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API UMainWidgetController : public UNGWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

};
