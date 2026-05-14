// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Pawn/NGPawnBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/NGPathFindingComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/NGPoolableComponent.h"
#include "GameModes/NGInGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"
#include "UI/NGWidgetInterface.h"

ANGPawnBase::ANGPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;		//네트워크 복제 활성화
	NetUpdateFrequency =66.f;
	// SetReplicatingMovement(true);	//위치 속도 복제 활성화
	AActor::SetReplicateMovement(true);
	
	PoolController = CreateDefaultSubobject<UNGPoolableComponent>(FName("PoolController"));
		
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComp"));
	RootComponent = CapsuleComponent;
    
	CapsuleComponent->InitCapsuleSize(40.f, 80.f);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));

	UnitMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("UnitMesh"));
	UnitMesh->SetupAttachment(RootComponent);
    
	UnitMesh->SetRelativeLocation(FVector(0.f, 0.f, -80.f));
	UnitMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	UnitMesh->SetReceivesDecals(false);
	
	AbilitySystemComponent = CreateDefaultSubobject<UNGAbilitySystemComponent>(TEXT("Ability System Component"));

	AttributeSet = CreateDefaultSubobject<UNGAttributeSet>(TEXT("AttributeSet"));
	
	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComponent->SetupAttachment(RootComponent);
	HPBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 10.f) + GetHalfCapsule());
	HPBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarComponent->SetDrawSize(FVector2D(100.f, 20.f));
	
	DetectionSphere = CreateDefaultSubobject<USphereComponent>(FName("DetectionSphere"));
	DetectionSphere->SetupAttachment(RootComponent);
	
	//ECC_Pawn에서 원하는 채널만 오버랩되도록 변경
	FVector HalfCap = GetHalfCapsule();
	FVector DetectionSphereLocation = DetectionSphere->GetComponentLocation();
	DetectionSphere->SetRelativeLocation(DetectionSphereLocation - HalfCap);
	
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANGPawnBase::OnDetectionBeginOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ANGPawnBase::OnDetectionEndOverlap);
	
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
	
	DOREPLIFETIME(ANGPawnBase, PlacedGridAddress);
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
	
	LocationOffset = GetHalfCapsule();
	
	if (AbilitySystemComponent)
	{
		InitializeAttributes();
		
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UNGAttributeSet::GetHealthAttribute()).AddUObject(this, &ANGPawnBase::OnHealthChanged);
	}
	
	if (AttributeSet)
	{
		if (DetectionSphere)
		{
			float CurrentRange = AttributeSet->GetAttackRange();
			DetectionSphere->SetSphereRadius(CurrentRange);
		}
	}
	
	UpdateHPBar();
	
}

void ANGPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (PawnState == EPawnState::Combat)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackCheckTimerHandle, this, &ANGUnitPawn::CheckAttackCondition, 0.2f, true);
		
		// if (PathFindingComponent)
		// {
		// 	TArray<FIntVector2> Path = PathFindingComponent->FindPathToClosestEnemy(PlacedGridAddress, OwnerIndex);
		// 	for (FIntVector2 PathIndex : Path)
		// 	{
		// 		FGridAddress NodeAddress(PathIndex, PlacedGridAddress.GridType, PlacedGridAddress.GridOwnerPS);
		// 		FVector WorldLocation = UGridMapHelper::GetWorldLocation(NodeAddress) + FVector(0.f, 0.f, 50.f);
		// 		DrawDebugSphere(
		// 			GetWorld(),             // 월드 컨텍스트
		// 			WorldLocation,          // 구의 중심 좌표
		// 			20.0f,                  // 반지름 (크기)
		// 			12,                     // 세그먼트 수 (구의 형태, 너무 높으면 프레임 드랍 발생)
		// 			FColor::Green,          // 색상 (경로는 초록색 추천)
		// 			false,                  // bPersistentLines: false로 해야 계속 안 쌓입니다.
		// 			-1.0f,                  // LifeTime: -1.0f로 두면 딱 다음 틱(1프레임)까지만 유지됩니다.
		// 			0,                      // DepthPriority: 0은 기본 렌더링 순서
		// 			2.0f                    // 선 두께
		// 		);
		// 	}
		// }
	}else
	{
		GetWorldTimerManager().ClearTimer(AttackCheckTimerHandle);
	}
}

void ANGPawnBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHPBar();
}

void ANGPawnBase::OnAttackRangeChanged(const FOnAttributeChangeData& Data)
{
	DetectionSphere->SetSphereRadius(Data.NewValue);
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

FVector ANGPawnBase::GetHalfCapsule() const
{
	FVector CapsuleHalfHeight = FVector::ZeroVector;

	CapsuleHalfHeight.Z = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	return CapsuleHalfHeight;
}

UAnimMontage* ANGPawnBase::GetAttackMontage() const
{
	return AttackMontage;
}

void ANGPawnBase::Die()
{
	FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	
	if (GetAbilitySystemComponent()->HasMatchingGameplayTag(DeadTag))	return;
	
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
	PrePlacedGridAddress = PlacedGridAddress;
	
	PlacedGridAddress = NewGridAddress;
}

void ANGPawnBase::Client_RejectMove_Implementation()
{		
	PlacedGridAddress = PrePlacedGridAddress;
	
	
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
	if (PlacedGridAddress.GridOwnerPS)
	{
		const FIntVector2 CombatGridIndex = UGridMapHelper::GetCellIndex(EGridType::Combat, TargetLocation, PlacedGridAddress.GridOwnerPS);
		FHexGridMap& CombatGridMap = PlacedGridAddress.GridOwnerPS->GetCombatGridMap();
		bool bCombatValidGrid = CombatGridMap.IsValidIndex(CombatGridIndex);
			
		FQuadGridMap& WaitGridMap = PlacedGridAddress.GridOwnerPS->GetWaitGridMap();
		const FIntVector2 WaitGridIndex = UGridMapHelper::GetCellIndex(EGridType::Wait, TargetLocation, PlacedGridAddress.GridOwnerPS);
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
	// 여기서 가도되는지 확인 후 서버에서 위치 옮기고 클라이언트한테 결과 전송

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
	UnSetPawnOnGrid(PlacedGridAddress);
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
		GridMap->EmptyGridMap(PlacedGridAddress.GridIndex);
	}
}

void ANGPawnBase::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// TODO: 이거 IsA말고 인터페이스로 하게 변경하는게 좋을듯
	if (OtherActor && OtherActor != this && OtherActor->IsA(ANGPawnBase::StaticClass()))
	{
		//TODO: 오너인덱스 확인
		InRangeTarget.AddUnique(Cast<ANGPawnBase>(OtherActor));
		
		UE_LOG(LogTemp, Log, TEXT("적 감지: %s"), *OtherActor->GetName());
	}
}

void ANGPawnBase::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		InRangeTarget.Remove(Cast<ANGPawnBase>(OtherActor));
		UE_LOG(LogTemp, Log, TEXT("적 사거리 이탈: %s"), *OtherActor->GetName());
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


void ANGPawnBase::ExecuteAttack()
{
	// if (CurrentTarget && AttackAbilitySpecHandle.IsValid())
	// {
	// 	AbilitySystemComponent->TryActivateAbility(AttackAbilitySpecHandle);		
	// }
	if (IsValid(CurrentTarget.Get()) && GetAbilitySystemComponent())
	{
		FGameplayEventData Payload;
		Payload.Instigator = this;
		Payload.Target = CurrentTarget;
		Payload.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(CurrentTarget);
		
		GetAbilitySystemComponent()->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(FName("Ability.Attack")), &Payload);
	}
}