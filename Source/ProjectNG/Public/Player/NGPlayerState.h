// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "Combat/Grid/Grid.h"
#include "GameFramework/PlayerState.h"
#include "NGPlayerState.generated.h"

class UNGPocketComponent;
/**
 * In-Game에서의 플레이어의 상태 정보를 저장하는 클래스
 *
 * 현재 보유 골드, 레벨, 경험치, 남은 체력 등 게임 판 내의 플레이어 정보를 저장하고 모든 클라이언트에게 복제(Replicate)할 때 사용
 * 플레이어가 구매하여 보유 중인 **유닛들의 목록(로스터)**을 관리
 */
UCLASS()
class PROJECTNG_API ANGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ANGPlayerState();
	
	//~Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	//~End IAbilitySystemInterface

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UNGAbilitySystemComponent* GetNGAbilitySystemComponent() const { return AbilitySystemComponent; }
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Grid")
	FHexGridMap GridMap;
	
protected:
	UPROPERTY()
	TObjectPtr<UNGAbilitySystemComponent> AbilitySystemComponent;
		
/*************************************/
/*				Pocket 관련			 */
/*************************************/
	
public:
	UNGPocketComponent* GetPlayerPocket() { return PlayerPocket; }

protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Game|Pocket")
	TObjectPtr<UNGPocketComponent> PlayerPocket;
	
};
