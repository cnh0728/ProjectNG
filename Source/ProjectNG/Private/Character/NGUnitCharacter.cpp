// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Character/NGUnitCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "AbilitySystem/NGGameplayAbility.h"
#include "Character/NGEnemyCharacter.h"
#include "Combat/GridMapManager.h"
#include "Combat/Weapon/NGWeaponData.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Core/NGGameplayTags.h"
#include "Game/NGGameState.h"
#include "GameModes/NGInGameGameMode.h"
#include "ProjectNG/ProjectNG.h"

// Sets default values
ANGUnitCharacter::ANGUnitCharacter() : AcceptanceRadius(1.0f), bIsGrabbed(false), bIsSelected(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
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
	
	DetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANGUnitCharacter::OnDetectionBeginOverlap);
	DetectionSphere->OnComponentEndOverlap.AddDynamic(this, &ANGUnitCharacter::OnDetectionEndOverlap);
	
	RangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecalNew"));
	RangeDecal->SetupAttachment(RootComponent);
	RangeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	ShowRangeIndicator(false);
}

void ANGUnitCharacter::OnSelected_Implementation()
{
	ISelectableInterface::OnSelected_Implementation();
	UE_LOG(LogTemp, Log, TEXT("OnSelected"));
	
	ShowRangeIndicator(true);
	
	bIsSelected = true;
}

void ANGUnitCharacter::OnDeselected_Implementation()
{
	ISelectableInterface::OnDeselected_Implementation();
	UE_LOG(LogTemp, Log, TEXT("OnDeselected"));
	
	ShowRangeIndicator(false);
	
	bIsSelected = false;
}

void ANGUnitCharacter::ShowRangeIndicator(bool bVisible) const
{
	UE_LOG(LogTemp, Warning, TEXT("Decal bVisible %s"), bVisible ? TEXT("On") : TEXT("Off"))
	if (RangeDecal)
	{
		RangeDecal->SetVisibility(bVisible);
	}
}

void ANGUnitCharacter::OnDrag_Implementation()
{
	ISelectableInterface::OnDrag_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnDrag"));
	
	bIsGrabbed = true;
}

void ANGUnitCharacter::OnUndrag_Implementation()
{
	ISelectableInterface::OnUndrag_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnUndrag"));
	
	bIsGrabbed = false;
}

void ANGUnitCharacter::Activate()
{
	InitAbilityActorInfo();

	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("ANGUnitCharacter::No attribute set"));
		return;
	}
	
	if (AbilitySystemComponent)
	{	        
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetAttackRangeAttribute()).AddUObject(this, &ANGUnitCharacter::OnAttackRangeChanged);
		
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
	
	GetWorld()->GetTimerManager().SetTimer(AttackCheckTimerHandle, this, &ANGUnitCharacter::CheckAttackCondition, 0.2f, true);
	
	Super::Activate();
}

void ANGUnitCharacter::Deactivate()
{
	GetWorld()->GetTimerManager().ClearTimer(AttackCheckTimerHandle);
	
	Super::Deactivate();
}

// Called when the game starts or when spawned
void ANGUnitCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshCache();
}

void ANGUnitCharacter::InitAbilityActorInfo()
{
	// ASC 복사 및 아바타 설정
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

// Called every frame
void ANGUnitCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsDragMoving && PlacedGridIndex.IsSet())
	{
		// MapManager Cache Late Allocation
		RefreshCache();
		
		FVector TargetLocation = MapManagerCache->GridMap.GetWorldLocation(PlacedGridIndex.GetValue()) + LocationOffset;
		
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

void ANGUnitCharacter::ExecuteAttack()
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
void ANGUnitCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANGUnitCharacter::SetDragTargetGridIndex(const TOptional<FIntVector2>& NewIndex)
{
	if (!MapManagerCache)	RefreshCache();
	
	if (!MapManagerCache->GridMap.IsGridIndexEmpty(NewIndex.GetValue()))
	{
		return;
	}
	
	SetPlacedGridIndex(NewIndex.GetValue());
	bIsDragMoving = true;
}

void ANGUnitCharacter::SetPlacedGridIndex(const FIntVector2& NewIndex)
{
	if (PlacedGridIndex.IsSet())
	{
		MapManagerCache->GridMap.EmptyGridMap(PlacedGridIndex.GetValue());
	}

	PlacedGridIndex = NewIndex;
	
	FGridData GridData;
	GridData.PlacedCharacter = this;
	
	MapManagerCache->GridMap.SetGridData(NewIndex, GridData);
}

TOptional<FIntVector2> ANGUnitCharacter::GetPlacedGridIndex()
{
	return PlacedGridIndex;
}

void ANGUnitCharacter::RefreshCache()
{
	if (!MapManagerCache)
	{
		if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>())
		{
			if (AGridMapManager* MapManager = GM->GetGridMapManager())
			{
				MapManagerCache = MapManager;
			}else
			{
				UE_LOG(LogTemp, Warning, TEXT("Can't Find MapManager"));
			}
		}
	}
}

void ANGUnitCharacter::UpdateDecalRange()
{
	float CurrentRange = AttributeSet->GetAttackRange();
	
	if (RangeDecal)
	{
		RangeDecal->DecalSize = FVector(500.f, CurrentRange, CurrentRange);
	}
}

void ANGUnitCharacter::OnAttackRangeChanged(const FOnAttributeChangeData& Data)
{
	Super::OnAttackRangeChanged(Data);
	
	DetectionSphere->SetSphereRadius(Data.NewValue);
}

void ANGUnitCharacter::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 이거 IsA말고 인터페이스로 하게 변경하는게 좋을듯
	if (OtherActor && OtherActor != this && OtherActor->IsA(ANGEnemyCharacter::StaticClass()))
	{
		DetectedTarget.AddUnique(Cast<ANGEnemyCharacter>(OtherActor));
		
		UE_LOG(LogTemp, Log, TEXT("적 감지: %s"), *OtherActor->GetName());
	}
}

void ANGUnitCharacter::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		DetectedTarget.Remove(Cast<ANGEnemyCharacter>(OtherActor));
		UE_LOG(LogTemp, Log, TEXT("적 사거리 이탈: %s"), *OtherActor->GetName());
	}
}

void ANGUnitCharacter::CheckAttackCondition()
{
	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack"))))
	{
		return;
	}
	
	ExecuteAttack();
}

void ANGUnitCharacter::EquipWeapon(UNGWeaponData* NewWeaponData)
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

void ANGUnitCharacter::InitializeAttributes()
{
	Super::InitializeAttributes();
	
	if (IsValid(AbilitySystemComponent))
	{
		UpdateDecalRange();
	}
}

