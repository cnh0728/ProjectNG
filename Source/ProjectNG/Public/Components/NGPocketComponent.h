// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NGPocketComponent.generated.h"

class ANGUnitPawn;

UENUM(BlueprintType)
enum class EShopActionType : uint8
{
	None,
	Roll,
	Buy,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTNG_API UNGPocketComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNGPocketComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override; // Server

	UFUNCTION(BlueprintCallable, Category = "Game|Shop")
	void RequestRoll();

	UFUNCTION(BlueprintPure, Category = "Game|Shop")
	const TArray<FName>& GetRollPocket() const { return RollShopPocket; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Shop")
	int32 PlayerLevel = 1; // TODO. 어빌리티 시스템으로 처리해야함. 현재 디버깅용으로 임시 변수

	void AddUnitToBuyingPocket(FName UnitName);

protected:
	UFUNCTION(Server, Reliable)
	void Server_RequestRoll();
	
	UFUNCTION()
	void OnRep_RollPocket();
	
	UFUNCTION()
	void UpdateRollUnit();

	UPROPERTY(EditDefaultsOnly, Category = "Game|Shop")
	int32 ShopSlotCount = 3;

private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_RollPocket, Category = "Game|Shop")
	TArray<FName> RollShopPocket; // 플레이어의 상점에 표시된 유닛 목록
	
	UPROPERTY(Replicated, VisibleAnywhere)
	EShopActionType LastShopAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game|Shop")
	TObjectPtr<UDataTable> ProbabilityTable;


public:
	TArray<ANGUnitPawn*> GetOwnedUnits() { return OwnedUnitPocket; };
	void GetPlacedUnits(TArray<ANGUnitPawn*>& OutUnits);
	
	void ControlPocketSpawning(ANGUnitPawn* NewPawn);
	void ControlPocketSelling(ANGUnitPawn* NewPawn);
	
	TArray<ANGUnitPawn*> GetOwnedUnitPocket()	{return OwnedUnitPocket; }
	
private:
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Game|Unit")
	TArray<ANGUnitPawn*> OwnedUnitPocket;
	
/*************************************/
/*				Debug용				 */
/*************************************/
public:
	UFUNCTION(BlueprintCallable, Category = "Debug")
	FString GetDebugNetMode() const
	{
		ENetMode NetMode = GetNetMode();
		switch (NetMode)
		{
		case NM_Client: return TEXT("Client");
		case NM_ListenServer: return TEXT("ListenServer");
		case NM_DedicatedServer: return TEXT("DedicatedServer");
		default: return TEXT("Standalone");
		}
	}
	
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void Debug_CheckBinding(UObject* WidgetRef) {
		UE_LOG(LogTemp, Warning, TEXT("[Bind] Pocket Addr: %p, Owner Addr: %p"), this, WidgetRef);
	}
};
