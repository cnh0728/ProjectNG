// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGUnitPawn.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "AbilitySystem/NGGameplayAbility.h"
#include "Pawn/NGEnemyPawn.h"
#include "Combat/Weapon/NGWeaponData.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Game/NGGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"

// Sets default values
ANGUnitPawn::ANGUnitPawn() : AcceptanceRadius(1.0f), bIsGrabbed(false), bIsSelected(false), bIsDragMoving(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	AActor::SetReplicateMovement(true);
	NetUpdateFrequency =66.f;
	
	if (!DetectionSphere)
	{
		DetectionSphere = CreateDefaultSubobject<USphereComponent>(FName("DetectionSphere"));
	}
	
	DetectionSphere->SetupAttachment(RootComponent);
	
	FVector HalfCap = GetHalfCapsule();
	FVector DetectionSphereLocation = DetectionSphere->GetComponentLocation();
	DetectionSphere->SetRelativeLocation(DetectionSphereLocation - HalfCap);
	
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	//ECC_Pawn에서 원하는 채널만 오버랩되도록 변경
	DetectionSphere->SetCollisionResponseToChannel(ECC_Enemy, ECR_Overlap);
	
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANGUnitPawn::OnDetectionBeginOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ANGUnitPawn::OnDetectionEndOverlap);
	
	RangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecalNew"));
	RangeDecal->SetupAttachment(RootComponent);
	RangeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	
	
	if (UnitMesh)
	{
		UnitMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		UnitMesh->SetCollisionResponseToChannel(ECC_SelectableUnit, ECR_Block);
	}
	
	ShowRangeIndicator(false);
}

void ANGUnitPawn::OnSelected_Implementation()
{
	ISelectableInterface::OnSelected_Implementation();
	UE_LOG(LogTemp, Log, TEXT("OnSelected"));
	
	ShowRangeIndicator(true);
	
	bIsSelected = true;
}

void ANGUnitPawn::OnDeselected_Implementation()
{
	ISelectableInterface::OnDeselected_Implementation();
	UE_LOG(LogTemp, Log, TEXT("OnDeselected"));
	
	ShowRangeIndicator(false);
	
	bIsSelected = false;
}

void ANGUnitPawn::ShowRangeIndicator(bool bVisible) const
{
	UE_LOG(LogTemp, Warning, TEXT("Decal bVisible %s"), bVisible ? TEXT("On") : TEXT("Off"));
	if (RangeDecal)
	{
		RangeDecal->SetVisibility(bVisible);
	}
}

void ANGUnitPawn::OnDrag_Implementation()
{
	ISelectableInterface::OnDrag_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnDrag"));
	
	bIsGrabbed = true;
}

void ANGUnitPawn::OnUndrag_Implementation()
{
	ISelectableInterface::OnUndrag_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnUndrag"));
	
	bIsGrabbed = false;
}

void ANGUnitPawn::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANGUnitPawn, bIsDragMoving);
	DOREPLIFETIME(ANGUnitPawn, PlacedGridAddress);
}

// Called when the game starts or when spawned
void ANGUnitPawn::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();
	
	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("ANGUnitCharacter::No attribute set"));
		return;
	}
	
	if (AbilitySystemComponent)
	{	        
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetAttackRangeAttribute()).AddUObject(this, &ANGUnitPawn::OnAttackRangeChanged);
		
		AttackAbilitySpecHandle = AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(AttackAbilityClass, 1, INDEX_NONE, this)
		);
	}
	
	float CurrentRange = AttributeSet->GetAttackRange();
	
	if (RangeDecal)
	{
		UpdateDecalRange();
		
		RangeDecal->SetDecalMaterial(RangeMaterial);
	}
	
	if (DetectionSphere)
	{
		DetectionSphere->SetSphereRadius(CurrentRange);
	}
	
	GetWorld()->GetTimerManager().SetTimer(AttackCheckTimerHandle, this, &ANGUnitPawn::CheckAttackCondition, 0.2f, true);
}

void ANGUnitPawn::InitAbilityActorInfo()
{
	// ASC 복사 및 아바타 설정
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

// Called every frame
void ANGUnitPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsDragMoving)
	{
		FVector TargetLocation = UGridMapHelper::GetWorldLocation(PlacedGridAddress) + LocationOffset;
		
		FVector CurrentLocation = GetActorLocation();
		
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, DragInterpSpeed);
		SetActorLocation(NewLocation);

		if (CurrentLocation.Equals(TargetLocation, AcceptanceRadius))
		{
			bIsDragMoving = false;
		}			
	}else
	{
		//드래그 이동중, 전투x
		if (DetectedTarget.Num() > 0 && IsValid(DetectedTarget[0]))
		{
			CurrentTarget = DetectedTarget[0];
			
			FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
			FRotator TargetRotation = Direction.Rotation();
			
			//Yaw만 사용
			TargetRotation.Pitch = 0.f;
			TargetRotation.Roll = 0.f;
			
			FRotator NewRotation = FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, RotationInterpSpeed);
			SetActorRotation(NewRotation);
		}else
		{
			CurrentTarget = nullptr;
		}
	}
}

void ANGUnitPawn::ExecuteAttack()
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

// Called to bind functionality to input
void ANGUnitPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANGUnitPawn::MoveTo(const FVector& TargetLocation)
{
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
			UE_LOG(LogTemp, Error, TEXT("ANGUnitPawn::MoveTo Cannot Place GridMap!"));
			return;
		}
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("ANGUnitPawn::MoveTo Cannot Find GridMap!"));
	}
	
	
	UpdatePlacedGridInfo(NewGridAddress);

	Request_MoveGrid(TargetLocation, NewGridAddress);
}

void ANGUnitPawn::UpdatePlacedGridInfo(FGridAddress NewGridAddress)
{
	PrePlacedGridAddress = PlacedGridAddress;
	
	bIsDragMoving = true;
	PlacedGridAddress = NewGridAddress;
}

void ANGUnitPawn::Client_RejectMove_Implementation()
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

EGridType ANGUnitPawn::GetCurrentGridType(const FVector& TargetLocation) const
{
	if (PlacedGridAddress.GridOwnerPS)
	{
		const FIntVector2 CombatGridIndex = UGridMapHelper::GetCellIndex(EGridType::Combat, TargetLocation, PlacedGridAddress.GridOwnerPS);
		FHexGridMap& CombatGridMap = PlacedGridAddress.GridOwnerPS->GetCombatGridMap();
		bool bCombatValidGrid = CombatGridMap.IsValidIndex(CombatGridIndex);
			
		FQuadGridMap& WaitGridMap = PlacedGridAddress.GridOwnerPS->GetWaitGridMap();
		const FIntVector2 WaitGridIndex = UGridMapHelper::GetCellIndex(EGridType::Wait, TargetLocation, PlacedGridAddress.GridOwnerPS);
		bool bWaitValidGrid = WaitGridMap.IsValidIndex(WaitGridIndex);
			
		if (bCombatValidGrid)
		{
			return EGridType::Combat;
		} if (bWaitValidGrid)
		{
			return EGridType::Wait;
		}
	}
	
	return EGridType::None;
}

bool ANGUnitPawn::CanPlaceUnit(FGridMapBase& GridMap, FIntVector2 GridIndex)
{
	return GridMap.IsGridIndexEmpty(GridIndex) && GridMap.IsValidIndex(GridIndex);
}

void ANGUnitPawn::Request_MoveGrid_Implementation(const FVector& TargetLocation, FGridAddress GridAddress)
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

void ANGUnitPawn::MovePawnOnGrid(const FGridAddress& GridAddress)
{
	UnSetPawnOnGrid(GridAddress);
	SetPawnOnGrid(GridAddress);
}

void ANGUnitPawn::SetPawnOnGrid(const FGridAddress& GridAddress)
{
	if (!HasAuthority())	return;
	
	FGridData GridData;
	GridData.PlacedPawn = this;
	
	UpdatePlacedGridInfo(GridAddress);
	
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(GridAddress))
	{
		GridMap->SetGridData(GridAddress.GridIndex, GridData);
	}
}

void ANGUnitPawn::UnSetPawnOnGrid(const FGridAddress& GridAddress) const
{
	if (!HasAuthority())	return;
	
	if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(GridAddress))
	{
		GridMap->EmptyGridMap(GridAddress.GridIndex);
	}
}

void ANGUnitPawn::UpdateDecalRange()
{
	float CurrentRange = AttributeSet->GetAttackRange();
	
	if (RangeDecal)
	{
		RangeDecal->DecalSize = FVector(500.f, CurrentRange, CurrentRange);
	}
}

void ANGUnitPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void ANGUnitPawn::OnAttackRangeChanged(const FOnAttributeChangeData& Data)
{
	Super::OnAttackRangeChanged(Data);
	
	DetectionSphere->SetSphereRadius(Data.NewValue);
}

void ANGUnitPawn::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// TODO: 이거 IsA말고 인터페이스로 하게 변경하는게 좋을듯
	if (OtherActor && OtherActor != this && OtherActor->IsA(ANGEnemyPawn::StaticClass()))
	{
		DetectedTarget.AddUnique(Cast<ANGEnemyPawn>(OtherActor));
		
		UE_LOG(LogTemp, Log, TEXT("적 감지: %s"), *OtherActor->GetName());
	}
}

void ANGUnitPawn::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		DetectedTarget.Remove(Cast<ANGEnemyPawn>(OtherActor));
		UE_LOG(LogTemp, Log, TEXT("적 사거리 이탈: %s"), *OtherActor->GetName());
	}
}

void ANGUnitPawn::CheckAttackCondition()
{
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack"))))
	{
		return;
	}
	
	ExecuteAttack();
}

void ANGUnitPawn::EquipWeapon(UNGWeaponData* NewWeaponData)
{
	if (!NewWeaponData || !GetAbilitySystemComponent()) return;
	
	if (CurrentWeaponAbilityHandle.IsValid())
	{
		GetAbilitySystemComponent()->ClearAbility(CurrentWeaponAbilityHandle);
	}
	
	if (NewWeaponData->WeaponAbilityClass)
	{
		CurrentWeaponAbilityHandle = GetAbilitySystemComponent()->GiveAbility(
			FGameplayAbilitySpec(NewWeaponData->WeaponAbilityClass, 1, static_cast<int32>(EAbilityInputID::Attack))
		);
	}
	
	UE_LOG(LogTemp, Log, TEXT("Weapon Swapped: Ability Replaced!"));
}

void ANGUnitPawn::InitializeAttributes()
{
	Super::InitializeAttributes();
	
	if (IsValid(AbilitySystemComponent))
	{
		UpdateDecalRange();
	}
}

