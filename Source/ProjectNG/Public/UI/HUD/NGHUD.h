// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/NGWidgetController.h"
#include "NGHUD.generated.h"

class UNGUnitInfoWidget;
class UUnitDetailsWidgetController;
class UNGUnitAttributeInfoDataAsset;
class UNGRollShopWidgetController;
class UNGUserWidget;

UCLASS()
class PROJECTNG_API ANGHUD : public AHUD
{
    GENERATED_BODY()

public:
    // 🎯 헤더와 cpp의 시그니처를 완전히 일치시킵니다.
    UNGRollShopWidgetController* CreateRollShopWidgetController(const FWidgetParams& WidgetControllerParams);
    UUnitDetailsWidgetController* CreateUnitDetailsWidgetController(const FWidgetParams& WidgetControllerParams);

    UNGRollShopWidgetController* GetRollShopWidgetController() { return RollShopWidgetController; }
    UUnitDetailsWidgetController* GetUnitDetailsWidgetController() { return UnitDetailsWidgetController; }
    UNGUnitInfoWidget* GetUnitInfoWidget() { return UnitInfoWidget; }
    
    void InitializeHUD(APlayerController* PC, APlayerState* PS);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WidgetData")
    TObjectPtr<UNGUnitAttributeInfoDataAsset> AttributeInfo;
    
private:
    //~ Begin Main Widget
    UPROPERTY()
    TObjectPtr<UNGUserWidget> MainWidget;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUserWidget> MainWidgetClass;
    //~ End Main Widget

    //~ Begin RollShop Widget
    UPROPERTY()
    TObjectPtr<UNGRollShopWidgetController> RollShopWidgetController;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UNGRollShopWidgetController> RollShopWidgetControllerClass;
    //~ End RollShop Widget
    
    //~ Begin UnitDetail Widget
    UPROPERTY()
    TObjectPtr<UNGUnitInfoWidget> UnitInfoWidget;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UNGUnitInfoWidget> UnitInfoWidgetClass; 
    //~ End UnitDetail Widget
    
    // ~Begin UnitDetails WidgetController
    UPROPERTY()
    TObjectPtr<UUnitDetailsWidgetController> UnitDetailsWidgetController;
    
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UUnitDetailsWidgetController> UnitDetailsWidgetControllerClass;
    // ~End UnitDetails WidgetController
};