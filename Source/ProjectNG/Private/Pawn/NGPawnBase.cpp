// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Pawn/NGPawnBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/NGPathFindingComponent.h"
#include "Components/NGPocketComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/NGPoolableComponent.h"
#include "GameModes/NGInGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"
#include "UI/NGWidgetInterface.h"

ANGPawnBase::ANGPawnBase() : RotationInterpSpeed(10.f), SpeedScale(100.f)
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;		//네트워크 복제 활성화
	NetUpdateFrequency =66.f;
	// SetReplicatingMovement(true);	//위치 속도 복제 활성화
	AActor::SetReplicateMovement(true);
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	
	PoolController = CreateDefaultSubobject<UNGPoolableComponent>(FName("PoolController"));
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComp"));
	CapsuleComponent->SetupAttachment(RootComponent);
    
	CapsuleComponent->InitCapsuleSize(40.f, 80.f);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	
	FVector CapsuleLocation = CapsuleComponent->GetRelativeLocation();
	CapsuleLocation.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	CapsuleComponent->SetRelativeLocation(CapsuleLocation);
	
	UnitMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("UnitMesh"));
	UnitMesh->SetupAttachment(CapsuleComponent);
    
	UnitMesh->SetRelativeLocation(FVector(0.f, 0.f, -80.f));
	UnitMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	UnitMesh->SetReceivesDecals(false);
	
	AbilitySystemComponent = CreateDefaultSubobject<UNGAbilitySystemComponent>(TEXT("Ability System Component"));

	AttributeSet = CreateDefaultSubobject<UNGAttributeSet>(TEXT("AttributeSet"));
	
	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComponent->SetupAttachment(CapsuleComponent);
	HPBarComponent->SetRelativeLocation(FVector(0.f, -5.f, 10.f) + GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	HPBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarComponent->SetDrawSize(FVector2D(100.f, 20.f));
	
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	
	PathFindingComponent = CreateDefaultSubobject<UNGPathFindingComponent>(TEXT("PathFindingComp"));
}

UAbilitySystemComponent* ANGPawnBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ANGPawnBase::Multi_PlayMontage_Implementation(UAnimMontage* MontageToPlay)
{
	UE_LOG(LogTemp, Log, TEXT("Multi PlayMontage"));
	PlayAnimMontage(MontageToPlay);
}

float ANGPawnBase::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	UAnimInstance* AnimInstance = (UnitMesh) ? UnitMesh->GetAnimInstance() : nullptr;
    
	if (AnimMontage && AnimInstance)
	{
		if (AnimInstance->IsAnyMontagePlaying())
		{
			AnimInstance->Montage_Stop(0.2f);
		}
		
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		if (Duration > 0.f)
		{
			// 특정 섹션부터 시작해야 한다면 점프
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}
			return Duration;
		}
	}

	return 0.f;
}

void ANGPawnBase::StopAnimMontage(UAnimMontage* AnimMontage)
{
	UAnimInstance* AnimInstance = (UnitMesh) ? UnitMesh->GetAnimInstance() : nullptr;

	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		if (AnimMontage)
		{
			// 몽타주 설정에 저장된 기본 블렌드 아웃 시간을 사용하여 부드럽게 중지
			AnimInstance->Montage_Stop(AnimMontage->GetDefaultBlendOutTime(), AnimMontage);
		}
		else
		{
			// 현재 재생 중인 모든 몽타주를 0.25초 동안 서서히 멈춤
			AnimInstance->Montage_Stop(0.25f);
		}
	}
}

void ANGPawnBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGPawnBase, CurrentGridAddress);
	DOREPLIFETIME(ANGPawnBase, NextGridPoint);
	DOREPLIFETIME(ANGPawnBase, PawnState);
}

void ANGPawnBase::HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType,
                                    const FGameplayCueParameters& Parameters)
{
	IGameplayCueInterface::HandleGameplayCue(Self, GameplayCueTag, EventType, Parameters);
	
	if (GameplayCueTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Hit"))))
	{
		if (EventType == EGameplayCueEvent::Executed)
		{
			PlayAnimMontage(DamagedMontage);
		}
	}
}

void ANGPawnBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilityActorInfo();
}

void ANGPawnBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitAbilityActorInfo();
}

void ANGPawnBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		InitializeAttributes();
		
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetHealthAttribute()).AddUObject(this, &ANGPawnBase::OnHealthChanged);
		
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetAttackRangeAttribute()).AddUObject(this, &ANGPawnBase::OnAttackRangeChanged);
	}
	
	UpdateHPBar();
}

void ANGPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// VisualizePath();
	
	if (HasAuthority())
	{
		CheckCombatState();
	}
	
	if (!HasAuthority())
	{
		if (PawnState == EPawnState::Following)
		{
			VisualizeFollowing(DeltaTime);
		}else if (PawnState == EPawnState::Combat)
		{
			if (CurrentTarget)
			{
				FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
				FRotator TargetRotation = Direction.Rotation();
			
				//Yaw만 사용
				TargetRotation.Pitch = 0.f;
				TargetRotation.Roll = 0.f;
			
				FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationInterpSpeed);
				SetActorRotation(NewRotation);
			}
		}
	}
}

void ANGPawnBase::CheckCombatState()
{
	if (!HasAuthority())	return;
	
	if (PawnState == EPawnState::Combat)
	{
		//싸우다가 적이 범위를 벗어나거나 죽으면 Wait 
		if (!IsCurrentTargetInRange() || CurrentTarget->IsDead())
		{
			TransitionToState(EPawnState::Wait);
		}
	}else if (PawnState == EPawnState::Wait)
	{
		FindNewTarget();
	}
}

float ANGPawnBase::GetMoveSpeed() const
{
	float MoveSpeed = AttributeSet ? AttributeSet->GetMoveSpeed() : 1.f;
	return MoveSpeed * SpeedScale;	
}

void ANGPawnBase::ConsiderTransitionState()
{
	//찾아갈길이 없으면 바로 앞에 있단 뜻
	if (TargetPath.Num() == 0)
	{
		TransitionToState(EPawnState::Combat);
	}else
	{
		ForceTransitionToState(EPawnState::Following);
	}
}

void ANGPawnBase::OnReachedNextGrid()
{
	UE_LOG(LogTemp, Log, TEXT("OnReach index: %s"), *NextGridPoint.ToString());
	
	//도착시 우선 그리드 업데이트 및 현재 길찾기 상황변동 파악
	FGridMapBase* GridMap = UGridMapHelper::GetGridMap(CurrentGridAddress);
	if (!GridMap)	return;
	
	FGridAddress NextGridAddress = CurrentGridAddress;
	NextGridAddress.GridIndex = NextGridPoint;
	MovePawnOnGrid(NextGridAddress);
	
	//적에게 도착시 전투상태로 이전
	int32 AttackRange = AttributeSet ? AttributeSet->GetAttackRange() : 1;
	if (UGridMapHelper::GetDistance(CurrentGridAddress.GridIndex, CurrentTarget->CurrentGridAddress.GridIndex) <= AttackRange 
		|| ++CurrentPathIndex >= TargetPath.Num())
	{
		TransitionToState(EPawnState::Combat);
		return;
	}
	
	//적이 죽었으면 새로운 적 찾기
	if (CurrentTarget->IsDead())
	{
		FindNewTarget();
		return;
	}
	
	//쫓던 적이 이동했으면 다시 찾기
	if (CurrentTarget && CurrentTarget->GetGridAddress().GridIndex != TargetLastIndex)
	{
		ReFindPath();
		return;
	}
	
	FIntVector2 NextGrid = TargetPath[CurrentPathIndex];
	if (GridMap->GridInfo[GridMap->ConvertPointToIndex(NextGrid)].PlacedPawn != nullptr)
	{
		ReFindPath();
		return;
	}
	
	//아무일 없으면 길따라 가기
	SetNextGridPoint(NextGrid);
}

void ANGPawnBase::ReFindPath()
{
	PathFindingComponent->FindPath(CurrentGridAddress, CurrentTarget->GetGridAddress(), TargetPath);
	InitializeFindNewPath();
}

void ANGPawnBase::FindNewTarget()
{
	if (ANGPawnBase* NewTarget = PathFindingComponent->FindPathToClosestEnemy(CurrentGridAddress, OwnerIndex, TargetPath))
	{
		CurrentTarget = NewTarget;
		InitializeFindNewPath();
	}
}

void ANGPawnBase::InitializeFindNewPath()
{
	CurrentPathIndex = 0;
	TargetLastIndex = CurrentTarget->CurrentGridAddress.GridIndex;
	SetNextGridPoint(TargetPath[CurrentPathIndex]);
		
	ConsiderTransitionState();
}

void ANGPawnBase::ForceTransitionToState(EPawnState NewState)
{
	//강제로 None으로 만들어서 무조건 TransitionToState수행
	PawnState = EPawnState::None;
	
	TransitionToState(NewState);
}

void ANGPawnBase::TransitionToState(EPawnState NewState)
{
	if (PawnState == NewState)	return;
	
	OnExitCurrentState(PawnState);
	
	PawnState = NewState;
	
	OnEnterNewState(NewState);
}

void ANGPawnBase::OnApplyHardCrowdControl()
{
	GetWorld()->GetTimerManager().PauseTimer(AttackCheckTimerHandle);
	GetWorld()->GetTimerManager().PauseTimer(PredictGridReachingTimerHandle);
}

void ANGPawnBase::OnRemoveHardCrowdControl()
{
	GetWorld()->GetTimerManager().UnPauseTimer(AttackCheckTimerHandle);
	GetWorld()->GetTimerManager().UnPauseTimer(PredictGridReachingTimerHandle);
}

void ANGPawnBase::OnExitCurrentState(EPawnState RestState)
{
	if (RestState == EPawnState::Combat)
	{
		GetWorldTimerManager().ClearTimer(AttackCheckTimerHandle);
		CurrentTarget = nullptr;
	}else if (RestState == EPawnState::HardCrowdControl)
	{
		OnRemoveHardCrowdControl();
	}
}

void ANGPawnBase::OnEnterNewState(EPawnState EnteringState)
{
	if (EnteringState == EPawnState::Combat)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackCheckTimerHandle, this, &ANGUnitPawn::CheckAttackCondition, 0.2f, true);
	}else if (EnteringState == EPawnState::HardCrowdControl)
	{
		OnApplyHardCrowdControl();
	}
}

void ANGPawnBase::VisualizeFollowing(float DeltaTime)
{
	//클라이언트에서만 실행됨
	if (HasAuthority())	return;
	
	FVector CurrentLocation = GetActorLocation();
	float MoveSpeed = GetMoveSpeed();
	
	FGridAddress NextGridAddress = CurrentGridAddress;
	NextGridAddress.GridIndex = NextGridPoint;
	FVector NextGridLocation = UGridMapHelper::GetWorldLocation(NextGridAddress);
	
	FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, NextGridLocation, DeltaTime, MoveSpeed);
	SetActorLocation(NewLocation);
	
	FVector MoveDirection = (NextGridLocation - CurrentLocation).GetSafeNormal2D();
	if (!MoveDirection.IsNearlyZero())
	{
		FRotator TargetRotation = MoveDirection.Rotation();
		FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationInterpSpeed * MoveSpeed);
		SetActorRotation(NewRotation);
	}
	
	float DistanceSq = FVector::DistSquared(NextGridLocation, NewLocation);
	if (DistanceSq < 1.f)
	{
		SetActorLocation(NextGridLocation);
	}
}

void ANGPawnBase::SetNextGridPoint(FIntVector2 NewNextGridPoint)
{
	if (!HasAuthority()) return;
	
	if (TargetPath.Num() == 0 || CurrentPathIndex >= TargetPath.Num()) return;

	NextGridPoint = NewNextGridPoint;
	
	FVector CurrentLoc = GetActorLocation();
    
	FGridAddress NextGridAddress = CurrentGridAddress;
	NextGridAddress.GridIndex = NextGridPoint;
	FVector NextGridLoc = UGridMapHelper::GetWorldLocation(NextGridAddress);

	float ActualDistance = FVector::Distance(CurrentLoc, NextGridLoc);
    
	float MoveSpeed = GetMoveSpeed();
	if (MoveSpeed <= 0.f) MoveSpeed = 1.f; 

	float TimeToReach = ActualDistance / MoveSpeed;

	GetWorldTimerManager().SetTimer(PredictGridReachingTimerHandle, this, &ANGPawnBase::OnReachedNextGrid, TimeToReach, false);
}

bool ANGPawnBase::IsCurrentTargetInRange() const
{
	if (!IsValid(CurrentTarget))	return false;
		
	int32 Distance = UGridMapHelper::GetDistance(CurrentGridAddress.GridIndex, CurrentTarget->CurrentGridAddress.GridIndex);
	int32 AttackRange = AttributeSet ? AttributeSet->GetAttackRange() : 1;
	
	if (Distance > AttackRange)
	{
		return false;
	}
	
	return true;

}

void ANGPawnBase::CollectInRangeUnits(TArray<ANGPawnBase*>& OutEnemies)
{
	//TODO: 나중에 여러명 한번에 때리는거 있으면 이거 쓰기
	uint32 AttackRange = AttributeSet ? AttributeSet->GetAttackRange() : 1;
	
	TArray<FIntVector2> Neighbors;
	UGridMapHelper::GetHexNeighborNodesInRange(CurrentGridAddress.GridIndex, AttackRange, Neighbors);
	
	//Neighbors 순회하면서 
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(CurrentGridAddress))
	{
		for (FIntVector2 NeighborIndex : Neighbors)
		{
			if (!GridMap->IsValidIndex(NeighborIndex))	continue;
			
			uint32 ConvertIndex = GridMap->ConvertPointToIndex(NeighborIndex);
			ANGPawnBase* FindPawn = GridMap->GridInfo[ConvertIndex].PlacedPawn;
			if (IsValid(FindPawn))
			{
				OutEnemies.Add(FindPawn);
			}
		}
	}
}

void ANGPawnBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHPBar();
}

void ANGPawnBase::OnAttackRangeChanged(const FOnAttributeChangeData& Data)
{
}

void ANGPawnBase::InitializeAttributes()
{
	if (!DefaultAttributeTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("No AttributeTable"));
		return;
	}
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitStats(UNGAttributeSet::StaticClass(), DefaultAttributeTable);
	}
}

void ANGPawnBase::Initialize(ANGPlayerState* PS)
{
	OwnerIndex = PS->GetUserIndex();
}

UAnimMontage* ANGPawnBase::GetAttackMontage() const
{
	return AttackMontage;
}

void ANGPawnBase::Die()
{
	FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	
	if (GetAbilitySystemComponent()->HasMatchingGameplayTag(DeadTag))	return;
	
	UnSetPawnOnGrid(CurrentGridAddress);
	
	// AddLooseGameplayTag는 메모리 상에서만 일시적 태그를 붙일 때 유용
	GetAbilitySystemComponent()->AddLooseGameplayTag(DeadTag);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(true); //레그돌
	
	if (ANGInGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameMode>())
	{
		GM->ReportPawnDeath(this);
	}
	
	SetLifeSpan(1.0f);
	
	UE_LOG(LogTemp, Log, TEXT("%s has died"), *GetName());
}

bool ANGPawnBase::IsDead()
{
	if (AbilitySystemComponent)
	{
		float CurrentHP = AbilitySystemComponent->GetNumericAttribute(UNGAttributeSet::GetHealthAttribute());
		
		return CurrentHP <= 0.f;
	}
	
	return true;
}

ANGPawnBase* ANGPawnBase::GetCurrentTarget()
{
	return CurrentTarget;
}

void ANGPawnBase::TurnPawnState(EPawnState InPawnState)
{
	PawnState = InPawnState;
}

void ANGPawnBase::UpdateHPBar()
{
	if (UUserWidget* HPWidget = HPBarComponent->GetUserWidgetObject())
	{
		float CurrentHP = AbilitySystemComponent->GetNumericAttribute(UNGAttributeSet::GetHealthAttribute());
		float MaxHP = AbilitySystemComponent->GetNumericAttribute(UNGAttributeSet::GetMaxHealthAttribute());
		
		float Percent = (MaxHP > 0.f) ? CurrentHP / MaxHP : 0.f;
		
		INGWidgetInterface::Execute_UpdateHP(HPWidget, Percent);
		
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("Can't update HPBar"));
	}
}

bool ANGPawnBase::CanAddUnitOnCombatGrid(EGridType NewGridType)
{
	if (CurrentGridAddress.GridType == EGridType::Wait && NewGridType == EGridType::Combat)
	{
		TArray<ANGUnitPawn*> PlacedUnitPocket;
		CurrentGridAddress.GridOwnerPS->GetPlayerPocket()->GetPlacedUnits(PlacedUnitPocket);

		if (PlacedUnitPocket.Num() >= CurrentGridAddress.GridOwnerPS->GetPlayerLevel())
		{
			return false;
		}
	}
	return true;
}

void ANGPawnBase::MoveTo(const FVector& TargetLocation)
{
	// Client에서만 불려야함
	if (HasAuthority())	return;
	
	ANGPlayerController* PC = GetOwner<ANGPlayerController>();
	if (!PC)	return;
	
	ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>();
	if (!PS)	return;
	
	//Client에서 미리 움직이고 서버에서 못간다 판단하면 reject
	EGridType NewGridType = GetCurrentGridType(TargetLocation);
	FIntVector2 NewIndex = UGridMapHelper::GetCellIndex(NewGridType, TargetLocation, PS);
	FGridAddress NewGridAddress(NewIndex, NewGridType, PS);
	
	if (!CanAddUnitOnCombatGrid(NewGridAddress.GridType))
	{
		UE_LOG(LogTemp, Warning, TEXT("Full of CombatGrid"));
		return;
	}
	
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(NewGridAddress))
	{
		// UE_LOG(LogTemp, Log, TEXT("GridMap : %s %p, Index: %s"), NewGridType == EGridType::Combat ? TEXT("Combat") : TEXT("Wait"), GridMap, *NewIndex.ToString())
		if (!CanPlaceUnit(*GridMap, NewIndex))
		{
			return;
		}
		UpdatePlacedGridInfo(NewGridAddress);

		Server_MoveGrid(TargetLocation, NewGridAddress);
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("ANGPawnBase::MoveTo Cannot Find GridMap!"));
	}
}

void ANGPawnBase::UpdatePlacedGridInfo(FGridAddress NewGridAddress)
{
	PreGridAddress = CurrentGridAddress;
	
	CurrentGridAddress = NewGridAddress;
}

void ANGPawnBase::Client_RejectMove_Implementation()
{		
	CurrentGridAddress = PreGridAddress;
	
	
	// 나중에 이동하는거 드래그앤 드랍했을때로 바꾸면 이것도 손보기
	/*
	if (ANGPlayerController* PC = GetOwner<ANGPlayerController>())
	{
		if (ANGPlayerState* PS = PC->GetPlayerState<ANGPlayerState>())
		{
			FVector Location = UGridMapHelper::GetWorldLocation(
				UGridMapHelper::GetGridMap(PS, PlacedGridType), 
				PlacedGridIndex, PlacedGridType);
			SetActorLocation(Location + LocationOffset);
		}
	}
	*/
}

EGridType ANGPawnBase::GetCurrentGridType(const FVector& TargetLocation) const
{
	if (CurrentGridAddress.GridOwnerPS)
	{
		const FIntVector2 CombatGridIndex = UGridMapHelper::GetCellIndex(EGridType::Combat, TargetLocation, CurrentGridAddress.GridOwnerPS);
		FHexGridMap& CombatGridMap = CurrentGridAddress.GridOwnerPS->GetCombatGridMap();
		bool bCombatValidGrid = CombatGridMap.IsValidIndex(CombatGridIndex);
			
		FQuadGridMap& WaitGridMap = CurrentGridAddress.GridOwnerPS->GetWaitGridMap();
		const FIntVector2 WaitGridIndex = UGridMapHelper::GetCellIndex(EGridType::Wait, TargetLocation, CurrentGridAddress.GridOwnerPS);
		bool bWaitValidGrid = WaitGridMap.IsValidIndex(WaitGridIndex);
		
		if (!bWaitValidGrid && !bCombatValidGrid)
		{
			UE_LOG(LogTemp, Error, TEXT("GridType is None, CombatIndex: %s, WaitIndex: %s"), *CombatGridIndex.ToString(), *WaitGridIndex.ToString());
		}
		
		if (bCombatValidGrid)
		{
			return EGridType::Combat;
		} 
		if (bWaitValidGrid)
		{
			return EGridType::Wait;
		}
	}
	
	return EGridType::None;
}

bool ANGPawnBase::CanPlaceUnit(FGridMapBase& GridMap, FIntVector2 GridIndex)
{
	return GridMap.IsGridIndexEmpty(GridIndex) && GridMap.IsValidIndex(GridIndex);
}

void ANGPawnBase::Server_MoveGrid_Implementation(const FVector& TargetLocation, FGridAddress GridAddress)
{
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(GridAddress))
	{
		FIntVector2 NewIndex = UGridMapHelper::GetCellIndex(GridAddress.GridType, TargetLocation, GridAddress.GridOwnerPS);
		
		if (!CanPlaceUnit(*GridMap, NewIndex))
		{
			//클라이언트한테 언도 지시
			Client_RejectMove();
			return;
		}
		
		MovePawnOnGrid(GridAddress);
	}
}

void ANGPawnBase::MovePawnOnGrid(const FGridAddress& GridAddress)
{
	UnSetPawnOnGrid(CurrentGridAddress);
	SetPawnOnGrid(GridAddress);
}

void ANGPawnBase::SetPawnOnGrid(const FGridAddress& GridAddress)
{
	if (!HasAuthority())	return;
	
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(GridAddress))
	{
		FGridData GridData;
		GridData.PlacedPawn = this;

		UpdatePlacedGridInfo(GridAddress);
		GridMap->SetGridData(GridAddress.GridIndex, GridData);
	}
}

void ANGPawnBase::UnSetPawnOnGrid(const FGridAddress& GridAddress) const
{
	if (!HasAuthority())	return;
	
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(GridAddress))
	{
		GridMap->EmptyGridMap(CurrentGridAddress.GridIndex);
	}
}

void ANGPawnBase::CheckAttackCondition()
{
	//여기서 DetectedTarget에 적이 있으면 ExecuteAttack, 없으면 FindingPath 및 길따라가기
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack"))))
	{
		return;
	}
	
	ExecuteAttack();
}

void ANGPawnBase::OnRep_CurrentGridAddress()
{
	FVector Location = UGridMapHelper::GetWorldLocation(CurrentGridAddress);
	SetActorLocation(Location);
}

void ANGPawnBase::ExecuteAttack()
{
	if (IsValid(CurrentTarget.Get()) && GetAbilitySystemComponent())
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.Target = CurrentTarget;
		Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(CurrentTarget);
		
		GetAbilitySystemComponent()->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("Ability.Attack")), &Payload);
	}
}

void ANGPawnBase::VisualizePath()
{
	if (PathFindingComponent)
	{
		TArray<FIntVector2> Path;
		PathFindingComponent->FindPathToClosestEnemy(CurrentGridAddress, OwnerIndex, Path);
		for (FIntVector2 PathIndex : Path)
		{
			FGridAddress NodeAddress(PathIndex, CurrentGridAddress.GridType, CurrentGridAddress.GridOwnerPS);
			FVector WorldLocation = UGridMapHelper::GetWorldLocation(NodeAddress) + FVector(0.f, 0.f, 50.f);
			DrawDebugSphere(
				GetWorld(),             // 월드 컨텍스트
				WorldLocation,          // 구의 중심 좌표
				20.0f,                  // 반지름 (크기)
				12,                     // 세그먼트 수 (구의 형태, 너무 높으면 프레임 드랍 발생)
				FColor::Green,          // 색상 (경로는 초록색 추천)
				false,                  // bPersistentLines: false로 해야 계속 안 쌓입니다.
				-1.0f,                  // LifeTime: -1.0f로 두면 딱 다음 틱(1프레임)까지만 유지됩니다.
				0,                      // DepthPriority: 0은 기본 렌더링 순서
				2.0f                    // 선 두께
			);
		}
	}
}