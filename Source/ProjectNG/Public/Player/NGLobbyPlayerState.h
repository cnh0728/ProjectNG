// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NGLobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API ANGLobbyPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ANGLobbyPlayerState();
	
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "UserData")
	uint8 bReady:1;
};
