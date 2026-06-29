// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "UI/Map/NGMapScreenWidget.h"
#include "UI/Map/NGMapConnectionLayerWidget.h"
#include "UI/Map/NGMapNodeWidget.h"
#include "Map/MapNodeDataAsset.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UNGMapScreenWidget::BuildMapUI(const TArray<FMapNodeData>& MapData, UMapNodeDataAsset* DataAsset)
{
	if (!MapCanvas || !MapNodeWidgetClass || !DataAsset)
	{
		return;
	}

	MapCanvas->ClearChildren();
	CachedMapData = MapData;

	ConnectionLayerWidget = WidgetTree->ConstructWidget<UNGMapConnectionLayerWidget>(UNGMapConnectionLayerWidget::StaticClass());
	if (ConnectionLayerWidget)
	{
		ConnectionLayerWidget->SetConnectionData(CachedMapData, LineColor, LineThickness);
		ConnectionLayerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

		UCanvasPanelSlot* LineSlot = MapCanvas->AddChildToCanvas(ConnectionLayerWidget);
		if (LineSlot)
		{
			LineSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			LineSlot->SetOffsets(FMargin(0.0f));
			LineSlot->SetZOrder(0);
		}
	}

	for (const FMapNodeData& Node : MapData)
	{
		UNGMapNodeWidget* NodeWidget = CreateWidget<UNGMapNodeWidget>(this, MapNodeWidgetClass);
		if (!NodeWidget)
		{
			continue;
		}

		FMapNodeVisualData VisualData;
		if (DataAsset->GetMapNodeVisual(Node.NodeTag, VisualData))
		{
			NodeWidget->InitializeNode(Node, VisualData);
		}

		UCanvasPanelSlot* CanvasSlot = MapCanvas->AddChildToCanvas(NodeWidget);
		if (CanvasSlot)
		{
			CanvasSlot->SetAnchors(FAnchors(Node.Location.X, Node.Location.Y));
			CanvasSlot->SetPosition(FVector2D::ZeroVector);
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetAutoSize(true);
			CanvasSlot->SetZOrder(1);
		}
	}
}
