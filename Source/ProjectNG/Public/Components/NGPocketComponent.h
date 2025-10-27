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
	const TArray<TSubclassOf<AActor>>& GetRollPocket() const { return RollPocket; }

protected:
	UFUNCTION(Server, Reliable)
	void Server_RequestRoll();

	UPROPERTY(EditDefaultsOnly, Category = "Game|Shop")
	int32 ShopSlotCount = 5;

private:
	UPROPERTY(ReplicatedUsing = OnRep_RollPocket)
	TArray<TSubclassOf<AActor>> RollPocket;

	UFUNCTION()
	void OnRep_RollPocket();

	UPROPERTY(EditDefaultsOnly, Category = "Game|Shop")
	TObjectPtr<UDataTable> ProbabilityTable;
};
