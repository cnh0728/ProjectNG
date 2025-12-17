// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NGLobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API ANGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ANGLobbyPlayerController();
	
	virtual void BeginPlay() override;
	
	UFUNCTION(Server, Reliable)
	void SetUserData(const FString& PlayerName, bool bReady);
};
