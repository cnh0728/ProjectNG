// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/NGPlayerController.h"
#include "GridMapManager.generated.h"

class ANGPlayerState;

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
	void BuildMyGrid();
	
	UFUNCTION(Server, Reliable)
	void Server_UpdateCameraTransform(const FTransform& HomeCam, const FTransform& AwayCam);
	
	void BuildGridVisual(ANGPlayerState* PS);

	void Initialize(const FGridBuildData& BuildData, ANGPlayerState* InPS);
	
	const FTransform& GetHomeCameraTransform() const { return HomeCameraTransform; }
	const FTransform& GetAwayCameraTransform() const { return AwayCameraTransform; }
	
private:
	
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
	
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Grid", meta=(AllowPrivateAccess = "true"))
	FTransform HomeCameraTransform;
	
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Grid", meta=(AllowPrivateAccess = "true"))
	FTransform AwayCameraTransform;
	
	UPROPERTY(EditAnywhere, Category = "Grid", meta=(AllowPrivateAccess = "true"))
	float CameraPitchAngle;

	UPROPERTY(EditAnywhere, Category = "Grid", meta=(AllowPrivateAccess = "true"))
	FVector CameraOffset;
	
	//PC는 자기꺼 아니면 null이라 참조못함(GetOwner불가) 그래서 PS를 따로 부여
	UPROPERTY(Replicated)
	ANGPlayerState* OwnerPS;
	
	FTimerHandle RetryTimerHandle;
	int32 RetryCount;
};
