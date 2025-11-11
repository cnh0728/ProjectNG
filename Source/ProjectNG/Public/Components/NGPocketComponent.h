// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NGPocketComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnitsUpdatedSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTNG_API UNGPocketComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNGPocketComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override; // Server

	UFUNCTION(BlueprintCallable, Category = "Game|Shop")
	void RequestRoll();

	UPROPERTY(BlueprintAssignable, Category = "Game|Shop")
	FOnUnitsUpdatedSignature OnUnitsUpdated;

	UFUNCTION(BlueprintPure, Category = "Game|Shop")
	const TArray<FName>& GetRollPocket() const { return RollPocket; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Shop")
	int32 PlayerLevel = 1; // TODO. 어빌리티 시스템으로 처리해야함. 현재 디버깅용으로 임시 변수

	void AddUnitToBuyingPocket(FName UnitName);

protected:
	UFUNCTION(Server, Reliable)
	void Server_RequestRoll();

	UPROPERTY(EditDefaultsOnly, Category = "Game|Shop")
	int32 ShopSlotCount = 3;

private:
	UPROPERTY(ReplicatedUsing = OnRep_RollPocket)
	TArray<FName> RollPocket; // 플레이어의 상점에 표시된 유닛 목록

	UPROPERTY(VisibleAnywhere, Replicated, Category = "Game|Shop")
	TArray<FName> PlayerUnitPocket; // 플레이어가 가진 유닛 목록

	UFUNCTION()
	void OnRep_RollPocket();
	
	UPROPERTY(EditDefaultsOnly, Category = "Game|Shop")
	TObjectPtr<UDataTable> ProbabilityTable;
};
