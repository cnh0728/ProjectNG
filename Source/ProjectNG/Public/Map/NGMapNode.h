// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NGMapTypes.h"
#include "GameFramework/Actor.h"
#include "Pawn/SelectableInterface.h"
#include "NGMapNode.generated.h"

UCLASS()
class PROJECTNG_API ANGMapNode : public AActor, public ISelectableInterface
{
	GENERATED_BODY()
	
public:
	ANGMapNode();

	void InitializeNode(const FMapNodeData& InNodeData);

	virtual void OnSelected_Implementation() override;
	
	virtual void OnDeselected_Implementation() override;

	const FMapNodeData& GetMapNodeData() const { return NodeData; }
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, Category = "MapNode")
	FMapNodeData NodeData;
};
