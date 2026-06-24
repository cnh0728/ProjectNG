// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "UI/Map/NGMapScreenWidget.h"
#include "UI/Map/NGMapNodeWidget.h"
#include "Map/MapNodeDataAsset.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"

void UNGMapScreenWidget::BuildMapUI(const TArray<FMapNodeData>& MapData, UMapNodeDataAsset* DataAsset)
{
	if (!MapCanvas || !MapNodeWidgetClass || !DataAsset) return;

	MapCanvas->ClearChildren();
	CachedMapData = MapData;

	for (const FMapNodeData& Node : MapData)
	{
		UNGMapNodeWidget* NodeWidget = CreateWidget<UNGMapNodeWidget>(this, MapNodeWidgetClass);
		if (NodeWidget)
		{
			FMapNodeVisualData VisualData;
			if (DataAsset->GetMapNodeVisual(Node.NodeTag, VisualData))
			{
				NodeWidget->InitializeNode(Node, VisualData);
			}

			UCanvasPanelSlot* CanvasSlot = MapCanvas->AddChildToCanvas(NodeWidget);
			if (CanvasSlot)
			{
				// Location은 0~1 비율이므로 CanvasSize에 곱해서 배치
				FVector2D Position = FVector2D(Node.Location.X * CanvasSize.X, Node.Location.Y * CanvasSize.Y);
				CanvasSlot->SetPosition(Position);
				
				// 위젯의 중심점 정렬 (0.5, 0.5)
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				CanvasSlot->SetAutoSize(true);
			}
		}
	}
}

int32 UNGMapScreenWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 MaxLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	
	if (CachedMapData.IsEmpty()) return MaxLayer;

	// 선을 그리기 위해 각 노드의 로컬 위치를 캐싱
	TMap<int32, FVector2D> NodePositions;
	for (const FMapNodeData& Node : CachedMapData)
	{
		FVector2D Pos = FVector2D(Node.Location.X * CanvasSize.X, Node.Location.Y * CanvasSize.Y);
		NodePositions.Add(Node.NodeID, Pos);
	}

	for (const FMapNodeData& Node : CachedMapData)
	{
		if (!NodePositions.Contains(Node.NodeID)) continue;

		FVector2D StartPos = NodePositions[Node.NodeID];

		for (int32 ConnectedID : Node.ConnectedNodeIDs)
		{
			// 양방향 중복 그리기 방지를 위해 ID 비교
			if (Node.NodeID < ConnectedID && NodePositions.Contains(ConnectedID))
			{
				FVector2D EndPos = NodePositions[ConnectedID];
				
				TArray<FVector2D> Points;
				Points.Add(StartPos);
				Points.Add(EndPos);

				FSlateDrawElement::MakeLines(
					OutDrawElements,
					MaxLayer,
					AllottedGeometry.ToPaintGeometry(),
					Points,
					ESlateDrawEffect::None,
					LineColor,
					true,
					LineThickness
				);
			}
		}
	}

	return MaxLayer + 1;
}
