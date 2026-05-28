// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Combat/Grid/Arena.h"

#include "Combat/Grid/Grid.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"
#include "Player/NGPlayerState.h"
#include "ProjectNG/ProjectNG.h"


AArena::AArena() : WaitGridOffsetLocation(200.f), CameraPitchAngle(-60.f), CameraOffset(FVector(-350.f, 700.f, 1200.f))
{
	bReplicates = true;
	
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	HexGridVisualComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexGridVisual"));
	HexGridVisualComponent->SetStaticMesh(HexMeshAsset);
	HexGridVisualComponent->NumCustomDataFloats = 1;
	HexGridVisualComponent->SetupAttachment(Root);
	
	QuadGridVisualComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("QuadGridVisual"));
	QuadGridVisualComponent->SetStaticMesh(QuadMeshAsset);
	QuadGridVisualComponent->NumCustomDataFloats = 1;
	QuadGridVisualComponent->SetupAttachment(Root);
}

void AArena::Initialize(const FGridBuildData& BuildData, ANGPlayerState* InPS)
{
	OwnerPS = InPS;
	
	InitGridMap(BuildData);
}

void AArena::BeginPlay()
{
	Super::BeginPlay();
	
}

void AArena::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AArena, GridBuildData);
	DOREPLIFETIME(AArena, HomeCameraTransform);
	DOREPLIFETIME(AArena, AwayCameraTransform);
	
}

void AArena::InitGridMap(const FGridBuildData& BuildData)
{
	const FVector MyLocation = GetActorLocation();
	
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
	if (PC)
	{
		if (ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>())
		{			
			FHexGridMap& CombatGridMap = PS->GetCombatGridMap();
			CombatGridMap.InitializeMap(BuildData.HexSizeX, BuildData.HexSizeY, BuildData.HexCellSize, MyLocation);
			
			FGridAddress CombatLowOffsetAddress(FIntVector2(0, 0), EGridType::Combat, PS, 0);
			FQuadGridMap& WaitGridMap = PS->GetWaitGridMap();
			FVector LowWaitLocation = FVector(UGridMapHelper::GetRelativeLocation(CombatLowOffsetAddress));
					
			WaitGridMap.InitializeMap(BuildData.QuadSizeX, BuildData.QuadSizeY, BuildData.QuadCellSize, MyLocation + FVector(LowWaitLocation.X - WaitGridOffsetLocation - WaitGridMap.Offset, 0.f, 0.f));

			FGridAddress CombatHighOffsetAddress(FIntVector2(CombatGridMap.Width - 1, CombatGridMap.Height - 1), EGridType::Combat, PS, 0);
			FQuadGridMap& EnemyWaitGridMap = PS->GetEnemyWaitGridMap();
			FVector HighWaitLocation = FVector(UGridMapHelper::GetRelativeLocation(CombatHighOffsetAddress));
			
			EnemyWaitGridMap.InitializeMap(BuildData.QuadSizeX, BuildData.QuadSizeY, BuildData.QuadCellSize, MyLocation + FVector(HighWaitLocation.X + WaitGridOffsetLocation - WaitGridMap.Offset, 0.f, 0.f));
			
			FVector HomeSpecPawnLocation = FVector(UGridMapHelper::GetWorldLocation(CombatLowOffsetAddress));
			FVector AwaySpecPawnLocation = FVector(UGridMapHelper::GetWorldLocation(CombatHighOffsetAddress));
	
			UE_LOG(LogTemp, Log, TEXT("SetCameraTransform"));
			
			HomeCameraTransform.SetLocation(HomeSpecPawnLocation + CameraOffset);
			HomeCameraTransform.SetRotation(FRotator(CameraPitchAngle, 0.f, 0.f).Quaternion());
	
			AwayCameraTransform.SetLocation(AwaySpecPawnLocation + FVector(-CameraOffset.X, -CameraOffset.Y, CameraOffset.Z));
			AwayCameraTransform.SetRotation(FRotator(CameraPitchAngle, 180.f, 0.f).Quaternion());
		}
		
		//OnRep_BuildGridVisual
		GridBuildData = BuildData;
	}

}

void AArena::OnRep_BuildGridVisual()
{	
	BuildMyGrid();
	
	//GridmapManager가 더이상 네트워크 패킷 교환 안한다는 뜻. 중간에 맵 변경해야하면 이거 지워야함
	SetNetDormancy(DORM_Initial);
}

void AArena::BuildMyGrid()
{
	if (++RetryCount > 10)
	{
		RetryCount = 0;
		UE_LOG(LogTemp, Warning, TEXT("Retry BuildGrid TimeOut"));
		return;
	}
	
	if (!OwnerPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("Retry BuildGrid PS is NULL"));
		GetWorldTimerManager().SetTimer(RetryTimerHandle, this, &AArena::BuildMyGrid, 0.1f, false);
		return;		
	}
	
	RetryCount = 0;
	UE_LOG(LogTemp, Log, TEXT("BuildGrid is Ready"));
	
	//그리드는 모두에게 보여야하기에 전부 생성
	BuildGridVisual(OwnerPS);
}

void AArena::BuildGridVisual(ANGPlayerState* PS)
{
	if (!HexGridVisualComponent || !QuadGridVisualComponent)	return;
	
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
	
	FHexGridMap& CombatGridMap = PS->GetCombatGridMap();
	for (int32 y=0; y<GridBuildData.HexSizeY; ++y)
	{
		for (int32 x=0; x<GridBuildData.HexSizeX; ++x)
		{
			FGridAddress CombatGridAddress(FIntVector2(x, y), EGridType::Combat, PS, 0);
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
	
	FGridAddress LowOffsetAddress(FIntVector2(0, 0), EGridType::Combat, PS, 0);
	FGridAddress HighOffsetAddress(FIntVector2(CombatGridMap.Width-1, CombatGridMap.Height-1), EGridType::Combat, PS, 0);
	
	FVector LowWaitLocationOffset = FVector(UGridMapHelper::GetRelativeLocation(LowOffsetAddress));
	FVector HighWaitLocationOffset = FVector(UGridMapHelper::GetRelativeLocation(HighOffsetAddress));

	for (int32 y=0; y<GridBuildData.QuadSizeY; ++y)
	{
		for (int32 x=0; x<GridBuildData.QuadSizeX; ++x)
		{			
			FQuadGridMap& WaitGridMap = PS->GetWaitGridMap();
			FGridAddress LowWaitGridAddress = FGridAddress(FIntVector2(x, y), EGridType::Wait, PS, 0);
			AddGridInstance(QuadGridVisualComponent, LowWaitGridAddress, FVector(LowWaitLocationOffset.X - WaitGridOffsetLocation - WaitGridMap.Offset, 0.f, 0.f));
					
			FQuadGridMap& EnemyWaitGridMap = PS->GetEnemyWaitGridMap();
			FGridAddress HighWaitGridAddress = FGridAddress(FIntVector2(x, y), EGridType::Wait, PS, 0);
			AddGridInstance(QuadGridVisualComponent, HighWaitGridAddress, FVector(HighWaitLocationOffset.X + WaitGridOffsetLocation - EnemyWaitGridMap.Offset, 0.f, 0.f));
		}
	}
	
	SetISMCCollision(QuadGridVisualComponent);
}

void AArena::HighlightAttackRange(const FGridAddress& PivotAddress, int32 AttackRange) const
{
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(PivotAddress))
	{
		TArray<FIntVector2> Neighbors;
		UGridMapHelper::GetHexNeighborIndexInRange(PivotAddress.GridIndex, AttackRange, Neighbors, GridMap);

		FGridAddress RangeAddress = PivotAddress;
		
		for (FIntVector2 Neighbor : Neighbors)
		{
			if (GridMap->IsValidIndex(Neighbor))
			{
				RangeAddress.GridIndex = Neighbor;
				HighlightSpecificGrid(RangeAddress, 0.1f, false);
			}
		}
	}
	
	HexGridVisualComponent->MarkRenderStateDirty();
}

void AArena::HighlightSpecificGrid(const FGridAddress& GridAddress, float HighlightFactor, bool bMarkRenderStateDirty) const
{
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(GridAddress))
	{
		int32 GridIndex = GridMap->ConvertPointToIndex(GridAddress.GridIndex);
		// UE_LOG(LogTemp, Log, TEXT("Highlight %s %f"), *GridAddress.GridIndex.ToString(), HighlightFactor);

		if (GridAddress.GridType == EGridType::Combat)
		{
			HexGridVisualComponent->SetCustomDataValue(GridIndex, 0, HighlightFactor, bMarkRenderStateDirty);
		}
		else if (GridAddress.GridType == EGridType::Wait){
			//아군꺼 만들고 적군꺼 만들고 반복해서 했기때문에 아군은 *2, 적군은 *2+1하면 됨
			QuadGridVisualComponent->SetCustomDataValue(GridIndex * 2, 0, HighlightFactor, bMarkRenderStateDirty);
		}
		else if (GridAddress.GridType == EGridType::EnemyWait)
		{
			QuadGridVisualComponent->SetCustomDataValue(GridIndex * 2 + 1, 0, HighlightFactor, bMarkRenderStateDirty);
		}
	}
}
