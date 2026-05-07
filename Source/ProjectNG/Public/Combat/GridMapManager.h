// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridMapManager.generated.h"

USTRUCT()
struct FGridBuildData{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	int32 SizeX;
	UPROPERTY(VisibleAnywhere)
	int32 SizeY;
	UPROPERTY(VisibleAnywhere)
	int32 CellSize;
	
	bool operator==(const FGridBuildData& Other) const {
		return SizeX == Other.SizeX && SizeY == Other.SizeY && CellSize == Other.CellSize;
	}
	bool operator!=(const FGridBuildData& Other) const {
		return !(*this == Other);
	}
	
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

	void InitGridMap(const int32 InitSizeX = 10, const int32 InitSizeY = 10, const double CellSize = 100.f);

public:
	UFUNCTION()
	void OnRep_BuildGridVisual();
	
	void Initialize(FGridBuildData BuildData);
	
private:
	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* HexGridVisualComponent;
	UPROPERTY(EditAnywhere, Category = "Visual")
	UStaticMesh* HexMeshAsset;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* QuadGridVisualComponent;
	UPROPERTY(EditAnywhere, Category = "Visual")
	UStaticMesh* QuadMeshAsset;
	
	// 디버그 라인 색상 등 시각화 관련 변수
	UPROPERTY(EditAnywhere, Category = "Visualization")
	FColor GridLineColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category = "Visualization")
	float LineThickness = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Spline")
	float SplineMarginFromEdge = 30.f;
	
	UPROPERTY(Replicated, VisibleAnywhere, ReplicatedUsing=OnRep_BuildGridVisual)
	FGridBuildData GridBuildData;
};
