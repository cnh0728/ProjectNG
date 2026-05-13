// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridMapManager.generated.h"

class ANGPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPSReady);

USTRUCT()
struct FGridBuildData{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	int32 HexSizeX;
	UPROPERTY(VisibleAnywhere)
	int32 HexSizeY;
	UPROPERTY(VisibleAnywhere)
	int32 HexCellSize;

	UPROPERTY(VisibleAnywhere)
	int32 QuadSizeX;
	UPROPERTY(VisibleAnywhere)
	int32 QuadSizeY;
	UPROPERTY(VisibleAnywhere)
	int32 QuadCellSize;
};

class USplineComponent;
/**
 * 
 */
UCLASS()
class PROJECTNG_API AGridMapManager : public AActor
{
	GENERATED_BODY()

public:
	AGridMapManager();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	void InitGridMap(const FGridBuildData& BuildData);

public:
	UFUNCTION()
	void OnRep_BuildGridVisual();
	
	void Initialize(const FGridBuildData& BuildData, uint32 OwnerIndex);
	
	UFUNCTION()
	void OnRep_OwnerPS();
	
	UFUNCTION()
	void SetOwnerPS(ANGPlayerState* InPS);
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPSReady OnPSReady;
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_OwnerPS)
	TObjectPtr<ANGPlayerState> OwnerPS;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* HexGridVisualComponent;
	UPROPERTY(EditAnywhere, Category = "Grid")
	UStaticMesh* HexMeshAsset;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* QuadGridVisualComponent;
	UPROPERTY(EditAnywhere, Category = "Grid")
	UStaticMesh* QuadMeshAsset;
	
	UPROPERTY(EditAnywhere, Category = "Grid", meta = (AllowPrivateAccess = "true"))
	float WaitGridOffsetLocation;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_BuildGridVisual)
	FGridBuildData GridBuildData;
	
	UPROPERTY(VisibleAnywhere)
	uint32 OwnerPlayerIndex;
};
