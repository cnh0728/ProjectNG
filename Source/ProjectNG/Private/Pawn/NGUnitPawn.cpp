// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGUnitPawn.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "AbilitySystem/NGGameplayAbility.h"
#include "Combat/Weapon/NGWeaponData.h"
#include "Components/DecalComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"

// Sets default values
ANGUnitPawn::ANGUnitPawn() : AcceptanceRadius(1.0f), bIsGrabbed(false), bIsSelected(false), bIsDragMoving(false)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
		
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
		AttackAbilitySpecHandle = AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(AttackAbilityClass, 1, INDEX_NONE, this)
		);
	}
	
	if (RangeDecal)
	{
		UpdateDecalRange();
		
		RangeDecal->SetDecalMaterial(RangeMaterial);
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

void ANGUnitPawn::UpdatePlacedGridInfo(FGridAddress NewGridAddress)
{
	Super::UpdatePlacedGridInfo(NewGridAddress);
	
	bIsDragMoving = true;
}

void ANGUnitPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
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
		// UE_LOG(LogTemp, Log, TEXT("Move to Grid Info"));
		
		FVector TargetLocation = UGridMapHelper::GetWorldLocation(CurrentGridAddress) + LocationOffset;
		
		FVector CurrentLocation = GetActorLocation();
		
		FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, DragInterpSpeed);
		SetActorLocation(NewLocation);

		if (CurrentLocation.Equals(TargetLocation, AcceptanceRadius))
		{
			bIsDragMoving = false;
		}			
	}else
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


// Called to bind functionality to input
void ANGUnitPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
