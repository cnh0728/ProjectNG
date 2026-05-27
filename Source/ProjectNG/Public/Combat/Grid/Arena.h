// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Arena.generated.h"

struct FGridAddress;
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
class PROJECTNG_API AArena : public AActor
{
	GENERATED_BODY()

public:
	AArena();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	void InitGridMap(const FGridBuildData& BuildData);

public:
	UFUNCTION()
	void OnRep_BuildGridVisual();
	void BuildMyGrid();
	
	void BuildGridVisual(ANGPlayerState* PS);
	void HighlightAttackRange(const FGridAddress& GridAddress,
	                          int32 AttackRange) const;
	void HighlightSpecificGrid(const FGridAddress& GridAddress, float HighlightFactor, bool bMarkRenderStateDirty) const;

	void Initialize(const FGridBuildData& BuildData, ANGPlayerState* InPS);
	
	ANGPlayerState* GetOwnerPS() {return OwnerPS;}
	
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
