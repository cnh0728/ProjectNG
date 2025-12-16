// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGHUD.h"
#include "UI/NGWidgetController.h"
#include "LobbyHUD.generated.h"

class UNGUserWidget;
class ULobbyWidgetController;
/**
 * 
 */
UCLASS()
class PROJECTNG_API ALobbyHUD : public ANGHUD
{
	GENERATED_BODY()
	
public:
	ULobbyWidgetController* GetMainWidgetController(const FWidgetParams& WidgetControllerParams);
	virtual void InitializeHUD(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS) override;
	
private:
	UPROPERTY()
	TObjectPtr<UNGUserWidget> MainWidget;
	
	UPROPERTY(EditDefaultsOnly, Category = "Lobby|Widget")
	TSubclassOf<UUserWidget> MainWidgetClass;
	
	UPROPERTY()
	TObjectPtr<ULobbyWidgetController> MainWidgetController;
	
	UPROPERTY(EditDefaultsOnly, Category = "Lobby|Widget")
	TSubclassOf<ULobbyWidgetController> MainWidgetControllerClass;
};
