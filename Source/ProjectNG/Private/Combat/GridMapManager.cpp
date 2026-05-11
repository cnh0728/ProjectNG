// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/GridMapManager.h"

#include "Combat/Grid/Grid.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"
#include "ProjectNG/ProjectNG.h"

AGridMapManager::AGridMapManager()
{
	bReplicates = true;
	
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	HexGridVisualComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexGridVisual"));
	HexGridVisualComponent->SetStaticMesh(HexMeshAsset);
	HexGridVisualComponent->SetupAttachment(Root);
	
	QuadGridVisualComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("QuadGridVisual"));
	QuadGridVisualComponent->SetStaticMesh(QuadMeshAsset);
	QuadGridVisualComponent->SetupAttachment(Root);
}

void AGridMapManager::Initialize(FGridBuildData BuildData)
{
	InitGridMap(BuildData.SizeX, BuildData.SizeY, BuildData.CellSize);
	
	if (ANGPlayerController* PC = GetOwner<ANGPlayerController>())
	{
		if (ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>())
		{
			FHexGridMap& CombatGridMap = PS->GetCombatGridMap();
		
			CombatGridMap.ResetEmptyGridIndex();
		}		
	}
	
}

void AGridMapManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGridMapManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGridMapManager, GridBuildData);
}

void AGridMapManager::InitGridMap(const int32 InitSizeX, const int32 InitSizeY, const double CellSize)
{
	const FVector MyLocation = GetActorLocation();
	
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
	if (PC)
	{
		if (ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>())
		{
			FHexGridMap& CombatGridMap = PS->GetCombatGridMap();
			CombatGridMap.InitializeMap(InitSizeX, InitSizeY, CellSize, MyLocation);
			CombatGridMap.ResetGridInfo();
		}
		
		GridBuildData = FGridBuildData(InitSizeX, InitSizeY, CellSize);
	}

}

void AGridMapManager::OnRep_BuildGridVisual()
{
	UE_LOG(LogTemp, Log, TEXT("BuildGridVisual Called"));
	
	if (!HexGridVisualComponent)	return;
	if (!QuadGridVisualComponent)	return;
	
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
	if (!PC)	return;
	
	if (ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>())
	{
		HexGridVisualComponent->ClearInstances();
		
		for (int32 y=0; y<GridBuildData.SizeY; ++y)
		{
			for (int32 x=0; x<GridBuildData.SizeX; ++x)
			{
				FHexGridMap& CombatGridMap = PS->GetCombatGridMap();
				FVector RelativeLoc = CombatGridMap.GetRelativeLocation(FIntVector2(x, y));
				FTransform InstanceTransform = FTransform(FRotator::ZeroRotator, RelativeLoc, FVector::OneVector);
				
				UE_LOG(LogTemp, Warning, TEXT("Tile Location: %s, Scale: %s"), *RelativeLoc.ToString(), *InstanceTransform.GetScale3D().ToString());
				HexGridVisualComponent->AddInstance(InstanceTransform);
			}
		}
		
		HexGridVisualComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HexGridVisualComponent->SetCollisionObjectType(ECC_WorldStatic);
		HexGridVisualComponent->SetCollisionProfileName(TEXT("Map"));
		// HexGridVisualComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		HexGridVisualComponent->SetCollisionResponseToChannel(ECC_Map, ECR_Block);
		
		HexGridVisualComponent->UpdateCollisionFromStaticMesh();
	}
	
	SetNetDormancy(DORM_Initial);
}
