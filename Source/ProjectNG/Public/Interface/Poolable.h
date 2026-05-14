// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Poolable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPoolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTNG_API IPoolable
{
	GENERATED_BODY()

public:
	virtual void Activate() = 0;
	virtual void Deactivate() = 0;
};
