// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "NGDeveloperSettings.generated.h"

class ANGCharacterBase;
class ANGProjectile;
/**
 * 
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "NG Pool Settings"))
class PROJECTNG_API UNGDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:	
	virtual FName GetContainerName() const override {return TEXT("Project");}
	virtual FName GetCategoryName() const override {return TEXT("Game");}
	virtual FName GetSectionName() const override {return TEXT("NGPoolSettings");}
	
	UPROPERTY(Config, EditAnywhere, Category = "NG Pool Settings")
	TMap<TSubclassOf<ANGProjectile>, TSoftClassPtr<ANGProjectile>> ProjectileClass;
	
	UPROPERTY(Config, EditAnywhere, Category = "NG Pool Settings")
	TMap<TSubclassOf<ANGCharacterBase>, TSoftClassPtr<ANGCharacterBase>> CharacterClass;
	
};
