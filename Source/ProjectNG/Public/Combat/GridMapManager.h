// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Grid/Grid.h"
#include "GridMapManager.generated.h"

class USplineComponent;
class ANGUnitPawn;
/**
 * 
 */
UCLASS()
class PROJECTNG_API AGridMapManager : public AActor
{
	GENERATED_BODY()

public:
	AGridMapManager();
	bool IsPossibleSpawnPawn() const;

	bool SpawnUnitPawn(FName UnitName, APlayerController* RequestingPlayer);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	void DrawGridLine();
	void MakeEnemySpline();

	void InitGridMap(const int32 InitSizeX = 10, const int32 InitSizeY = 10, const double CellSize = 100.f);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* EnemyPathSpline;

private:
	// 디버그 라인 색상 등 시각화 관련 변수
	UPROPERTY(EditAnywhere, Category = "Visualization")
	FColor GridLineColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category = "Visualization")
	float LineThickness = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Spline")
	float SplineMarginFromEdge = 30.f;
	
};
