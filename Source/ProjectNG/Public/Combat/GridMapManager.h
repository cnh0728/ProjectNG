// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Grid/Grid.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"
#include "GridMapManager.generated.h"

USTRUCT()
struct FGridBuildData{
	GENERATED_BODY()
	int32 SizeX;
	int32 SizeY;
	int32 CellSize;
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

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
	void DrawGridLine();
	void MakeEnemySpline();

	void InitGridMap(const int32 InitSizeX = 10, const int32 InitSizeY = 10, const double CellSize = 100.f);


public:
	void BuildGridVisual(int32 SizeX, int32 SizeY, float Margin);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* EnemyPathSpline;

	void Initialize(ANGPlayerController* InPC, FGridBuildData BuildData);
private:
	UPROPERTY(Transient)
	TObjectPtr<ANGPlayerController> OwnerPCCache;
	
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
	
};
