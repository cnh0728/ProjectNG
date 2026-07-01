// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "UI/Map/NGMapScreenWidget.h"
#include "UI/Map/NGMapConnectionLayerWidget.h"
#include "UI/Map/NGMapNodeWidget.h"
#include "Map/MapNodeDataAsset.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Game/NGGameState.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"

void UNGMapScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindGameFlowEvent();
}

void UNGMapScreenWidget::NativeDestruct()
{
	if (ANGGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ANGGameState>() : nullptr)
	{
		GameState->OnGameFlowChanged.RemoveDynamic(this, &ThisClass::HandleGameFlowChanged);
	}

	Super::NativeDestruct();
}

void UNGMapScreenWidget::BuildMapUI(const TArray<FMapNodeData>& MapData, UMapNodeDataAsset* DataAsset)
{
	if (!MapCanvas || !MapNodeWidgetClass || !DataAsset)
	{
		return;
	}

	MapCanvas->ClearChildren();
	CachedMapData = MapData;
	NodeWidgets.Reset();
	BindGameFlowEvent();

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
		DataAsset->GetMapNodeVisual(Node.NodeTag, VisualData);
		NodeWidget->InitializeNode(Node, VisualData);
		NodeWidget->OnNodeClicked.AddDynamic(this, &ThisClass::HandleNodeClicked);
		NodeWidgets.Add(Node.NodeID, NodeWidget);

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

	RefreshNodeAvailability();
}

void UNGMapScreenWidget::HandleNodeClicked(int32 NodeID)
{
	ANGPlayerController* PlayerController = GetOwningPlayer<ANGPlayerController>();
	if (!PlayerController || NodeID < 0)
	{
		return;
	}

	PlayerController->Server_SelectNode(NodeID);

	for (const TPair<int32, TObjectPtr<UNGMapNodeWidget>>& Pair : NodeWidgets)
	{
		if (Pair.Value)
		{
			Pair.Value->SetNodeSelectable(false);
		}
	}
}

void UNGMapScreenWidget::RefreshNodeAvailability()
{
	const ANGGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ANGGameState>() : nullptr;
	const ANGPlayerController* PlayerController = GetOwningPlayer<ANGPlayerController>();
	const ANGPlayerState* PlayerState = PlayerController ? PlayerController->GetPlayerState<ANGPlayerState>() : nullptr;

	for (const TPair<int32, TObjectPtr<UNGMapNodeWidget>>& Pair : NodeWidgets)
	{
		UNGMapNodeWidget* NodeWidget = Pair.Value;
		if (!NodeWidget)
		{
			continue;
		}

		bool bSelectable = false;
		if (GameState && PlayerState && !PlayerState->HasSelectedNode())
		{
			if (GameState->CurrentPhase == EGameplayPhase::TownSelection)
			{
				bSelectable = NodeWidget->NodeData.NodeType == ENodeType::Town;
			}
			else if (GameState->CurrentPhase == EGameplayPhase::NodeSelection)
			{
				const FMapNodeData* CurrentNode = GameState->MapNodes.FindByPredicate(
					[PlayerState](const FMapNodeData& Node)
					{
						return Node.NodeID == PlayerState->GetCurrentNodeID();
					});

				bSelectable = CurrentNode
					&& Pair.Key != PlayerState->GetCurrentNodeID()
					&& CurrentNode->ConnectedNodeIDs.Contains(Pair.Key);
			}
		}

		NodeWidget->SetNodeSelectable(bSelectable);
	}
}

void UNGMapScreenWidget::HandleGameFlowChanged(EGameplayPhase CurrentPhase, int32 CurrentTurn,
	float PhaseStartServerTime, float PhaseDuration, float RemainingTime)
{
	RefreshNodeAvailability();
}

void UNGMapScreenWidget::BindGameFlowEvent()
{
	if (ANGGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ANGGameState>() : nullptr)
	{
		GameState->OnGameFlowChanged.AddUniqueDynamic(this, &ThisClass::HandleGameFlowChanged);
	}
}
