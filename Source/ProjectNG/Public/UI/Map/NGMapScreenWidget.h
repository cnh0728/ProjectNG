// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/NGUserWidget.h"
#include "Map/NGMapTypes.h"
#include "NGMapScreenWidget.generated.h"

class UMapNodeDataAsset;
class UNGMapNodeWidget;
class UCanvasPanel;

/**
 * 맵 전체를 그리는 캔버스 화면 위젯
 */
UCLASS()
class PROJECTNG_API UNGMapScreenWidget : public UNGUserWidget
{
	GENERATED_BODY()
	
public:
	// 맵 데이터 기반으로 UI 생성
	UFUNCTION(BlueprintCallable, Category = "MapUI")
	void BuildMapUI(const TArray<FMapNodeData>& MapData, UMapNodeDataAsset* DataAsset);

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	// 캔버스 크기 (기본값 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MapUI")
	FVector2D CanvasSize = FVector2D(1920.f, 1080.f);

	// 라인 색상 및 굵기
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MapUI")
	FLinearColor LineColor = FLinearColor(1.f, 1.f, 1.f, 0.5f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MapUI")
	float LineThickness = 2.0f;

	// 노드 위젯 클래스 (블루프린트에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MapUI")
	TSubclassOf<UNGMapNodeWidget> MapNodeWidgetClass;

	// 생성된 노드 위젯들을 담는 컨테이너 패널
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MapCanvas;

private:
	// 연결선을 그리기 위해 노드 좌표를 저장
	TArray<FMapNodeData> CachedMapData;
};
