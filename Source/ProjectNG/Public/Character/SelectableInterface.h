// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SelectableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class USelectableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTNG_API ISelectableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Drag")
	void OnDrag();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Drag")
	void OnUndrag();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Selection")
	void OnSelected();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Selection")
	void OnDeselected();
	
};
