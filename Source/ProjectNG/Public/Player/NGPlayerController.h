// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NGPlayerController.generated.h"

class UNGPocketComponent;
/**
 * 플레이어의 입력을 처리하는 Class
 */
UCLASS()
class PROJECTNG_API ANGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ANGPlayerController();

	UNGPocketComponent* GetPlayerPocket() { return PlayerPocket; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game|Pocket")
	TObjectPtr<UNGPocketComponent> PlayerPocket;
};
