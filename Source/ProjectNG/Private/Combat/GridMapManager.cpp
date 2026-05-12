// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/GridMapManager.h"

#include "Combat/Grid/Grid.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"
#include "ProjectNG/ProjectNG.h"

AGridMapManager::AGridMapManager() : WaitGridOffsetLocation(200.f)
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

void AGridMapManager::Initialize(const FGridBuildData& BuildData, const uint32 OwnerIndex)
{
	OwnerPlayerIndex = OwnerIndex;
	
	InitGridMap(BuildData);
}

void AGridMapManager::OnRep_OwnerPS()
{
	if (OwnerPS)
	{
		UE_LOG(LogTemp, Log, TEXT("OnRep_OwnerPS Called"));
		
		OnPSReady.Broadcast();
		
		OnPSReady.Clear();
	}
}

void AGridMapManager::SetOwnerPS(ANGPlayerState* InPS)
{
	OwnerPS = InPS;
}

void AGridMapManager::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGridMapManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AGridMapManager, GridBuildData);
	DOREPLIFETIME(AGridMapManager, OwnerPS);
}

void AGridMapManager::InitGridMap(const FGridBuildData& BuildData)
{
	const FVector MyLocation = GetActorLocation();
	
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
	if (PC)
	{
		if (ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>())
		{			
			FHexGridMap& CombatGridMap = PS->GetCombatGridMap();
			CombatGridMap.InitializeMap(BuildData.HexSizeX, BuildData.HexSizeY, BuildData.HexCellSize, MyLocation);
			
			FGridAddress CombatLowOffsetAddress(FIntVector2(0, 0), EGridType::Combat, PS);
			FQuadGridMap& WaitGridMap = PS->GetWaitGridMap();
			FVector LowWaitLocation = FVector(UGridMapHelper::GetRelativeLocation(CombatLowOffsetAddress));
					
			WaitGridMap.InitializeMap(BuildData.QuadSizeX, BuildData.QuadSizeY, BuildData.QuadCellSize, MyLocation + FVector(LowWaitLocation.X - WaitGridOffsetLocation - WaitGridMap.Offset, 0.f, 0.f));

			FGridAddress CombatHighOffsetAddress(FIntVector2(CombatGridMap.Width - 1, CombatGridMap.Height - 1), EGridType::Combat, PS);
			FQuadGridMap& EnemyWaitGridMap = OwnerPS->GetEnemyWaitGridMap();
			FVector HighWaitLocation = FVector(UGridMapHelper::GetRelativeLocation(CombatHighOffsetAddress));
			
			EnemyWaitGridMap.InitializeMap(BuildData.QuadSizeX, BuildData.QuadSizeY, BuildData.QuadCellSize, MyLocation + FVector(HighWaitLocation.X + WaitGridOffsetLocation - WaitGridMap.Offset, 0.f, 0.f));
		}
		
		//OnRep_BuildGridVisual
		GridBuildData = BuildData;
	}

}

void AGridMapManager::OnRep_BuildGridVisual()
{	
	if (!HexGridVisualComponent || !QuadGridVisualComponent)	return;
	
	if (!OwnerPS){
		UE_LOG(LogTemp, Log, TEXT("Delegate PSOnReady"));
		OnPSReady.AddUniqueDynamic(this, &AGridMapManager::OnRep_BuildGridVisual);
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("OnRep_BuildGridVisual Called PS is alive"));
	
	auto SetISMCCollision = [](UInstancedStaticMeshComponent* ISMC)
	{
		if (ISMC)
		{
			ISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			ISMC->SetCollisionObjectType(ECC_WorldStatic);
			ISMC->SetCollisionProfileName(TEXT("Map"));
			// ISMS->SetCollisionResponseToAllChannels(ECR_Ignore);
			ISMC->SetCollisionResponseToChannel(ECC_Map, ECR_Block);
			
			ISMC->UpdateCollisionFromStaticMesh();
		}
	};
	
	////////////////////////////
	/// Hex Grid Build
	HexGridVisualComponent->ClearInstances();
	
	FHexGridMap& CombatGridMap = OwnerPS->GetCombatGridMap();
	for (int32 y=0; y<GridBuildData.HexSizeY; ++y)
	{
		for (int32 x=0; x<GridBuildData.HexSizeX; ++x)
		{
			FGridAddress CombatGridAddress(FIntVector2(x, y), EGridType::Combat, OwnerPS);
			FVector RelativeLoc = UGridMapHelper::GetRelativeLocation(CombatGridAddress);
			FTransform HexInstanceTransform = FTransform(FRotator::ZeroRotator, RelativeLoc, FVector::OneVector);
					
			HexGridVisualComponent->AddInstance(HexInstanceTransform);
		}
	}
	
	SetISMCCollision(HexGridVisualComponent);
	
	auto AddGridInstance = [&](UInstancedStaticMeshComponent* ISMC, FGridAddress GridAddress, const FVector& Pivot)
	{
		FVector RelativeLoc = UGridMapHelper::GetRelativeLocation(GridAddress) + Pivot;
		FTransform InstanceTransform(FRotator::ZeroRotator, RelativeLoc, FVector::OneVector);
		ISMC->AddInstance(InstanceTransform);
	};
	
	
	QuadGridVisualComponent->ClearInstances();
	for (int32 y=0; y<GridBuildData.QuadSizeY; ++y)
	{
		for (int32 x=0; x<GridBuildData.QuadSizeX; ++x)
		{			
			FQuadGridMap& WaitGridMap = OwnerPS->GetWaitGridMap();
			FGridAddress LowOffsetAddress(FIntVector2(0, 0), EGridType::Combat, OwnerPS);
			FVector LowWaitLocationOffset = FVector(UGridMapHelper::GetRelativeLocation(LowOffsetAddress));
			FGridAddress LowWaitGridAddress = FGridAddress(FIntVector2(x, y), EGridType::Wait, OwnerPS);
			AddGridInstance(QuadGridVisualComponent, LowWaitGridAddress, FVector(LowWaitLocationOffset.X - WaitGridOffsetLocation - WaitGridMap.Offset, 0.f, 0.f));
					
			FQuadGridMap& EnemyWaitGridMap = OwnerPS->GetEnemyWaitGridMap();
			FGridAddress HighOffsetAddress(FIntVector2(CombatGridMap.Width-1, CombatGridMap.Height-1), EGridType::Combat, OwnerPS);
			FVector HighWaitLocationOffset = FVector(UGridMapHelper::GetRelativeLocation(HighOffsetAddress));
			FGridAddress HighWaitGridAddress = FGridAddress(FIntVector2(x, y), EGridType::Wait, OwnerPS);
			AddGridInstance(QuadGridVisualComponent, HighWaitGridAddress, FVector(HighWaitLocationOffset.X + WaitGridOffsetLocation - EnemyWaitGridMap.Offset, 0.f, 0.f));
		}
	}
	
	SetISMCCollision(QuadGridVisualComponent);
	
	//GridmapManager가 더이상 네트워크 패킷 교환 안한다는 뜻. 중간에 맵 변경해야하면 이거 지워야함
	SetNetDormancy(DORM_Initial);
}
