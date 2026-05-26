// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawn/NGUnitPawn.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "AbilitySystem/NGGameplayAbility.h"
#include "Combat/Grid/Arena.h"
#include "Combat/Weapon/NGWeaponData.h"
#include "Core/NGDeveloperSettings.h"
#include "Player/NGPlayerController.h"
#include "ProjectNG/ProjectNG.h"

// Sets default values
ANGUnitPawn::ANGUnitPawn() : AcceptanceRadius(1.0f), bIsGrabbed(false), bIsSelected(false)
{
	PrimaryActorTick.bCanEverTick = false;

	if (UnitMesh)
	{
		UnitMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		UnitMesh->SetCollisionResponseToChannel(ECC_SelectableUnit, ECR_Block);
	}
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
}

void ANGUnitPawn::OnSelected_Implementation()
{
	ISelectableInterface::OnSelected_Implementation();
	UE_LOG(LogTemp, Log, TEXT("OnSelected"));
	
	//TODO: GridAddress 인자 받아와서 RangeIndicator 마저 완성하기
	// ShowRangeIndicator(true, );
	
	bIsSelected = true;
}

void ANGUnitPawn::OnDeselected_Implementation()
{
	ISelectableInterface::OnDeselected_Implementation();
	UE_LOG(LogTemp, Log, TEXT("OnDeselected"));
	
	// ShowRangeIndicator(false, TODO);
	
	bIsSelected = false;
}

void ANGUnitPawn::ShowRangeIndicator(bool bVisible, FGridAddress PivotAddress) const
{
	UE_LOG(LogTemp, Warning, TEXT("Decal bVisible %s"), bVisible ? TEXT("On") : TEXT("Off"));
	
	
	int32 HighlightRange = AttributeSet ? AttributeSet->GetAttackRange() : 1;
	//대기석은 자기위치만
	if (PivotAddress.GridType != EGridType::Combat)
	{
		HighlightRange = 0;
	}
	
	TArray<FIntVector2> Neighbors;
	UGridMapHelper::GetHexNeighborIndexInRange(PivotAddress.GridIndex, HighlightRange, Neighbors);
	
	//이 유닛이 현재 있는 그리드에서 근처 노드를 가져온 다음에, 그 노드들 인덱스에 맞게 Arena에 있는 것들 GridVisualComponent 켜주기
	if (AArena* CombatArena = PivotAddress.GridOwnerPS->GetHomeArena())
	{
		if (FGridMapBase* GridMap = UGridMapHelper::GetGridMap(PivotAddress))
		{
			for (FIntVector2 Neighbor : Neighbors)
			{
				if (GridMap->IsValidIndex(Neighbor))
				{
					CombatArena->HighlightSpecificGrid(PivotAddress, true);
				}
			}
		}
	}
}

void ANGUnitPawn::OnDrag_Implementation()
{
	ISelectableInterface::OnDrag_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnDrag"));
	//잡힌 모션 -> 레그돌화 해도 ㄱㅊ을듯
	bIsGrabbed = true;
}

void ANGUnitPawn::OnUndrag_Implementation()
{
	ISelectableInterface::OnUndrag_Implementation();

	UE_LOG(LogTemp, Log, TEXT("OnUndrag"));
	
	bIsGrabbed = false;
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


// Called to bind functionality to input
void ANGUnitPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
