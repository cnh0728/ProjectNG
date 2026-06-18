// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Map/NGMapNode.h"

ANGMapNode::ANGMapNode()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	SetRootComponent(StaticMesh);
}

void ANGMapNode::InitializeNode(const FMapNodeData& InNodeData)
{
	NodeData = InNodeData;
}

void ANGMapNode::OnSelected_Implementation()
{
	// 이펙트 효과 or 표시할 수 있는 거
}

void ANGMapNode::OnDeselected_Implementation()
{
	// 효과 제거
}

