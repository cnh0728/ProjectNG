// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "NGAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNG_API UNGAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static UNGAssetManager& Get();
	
protected:
	virtual void StartInitialLoading() override;
};
