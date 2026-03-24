// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Character/NGCharacterBase.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Character/NGUnitCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameModes/NGCombatGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/OutputDeviceNull.h"
#include "UI/NGWidgetInterface.h"


ANGCharacterBase::ANGCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	if (!AbilitySystemComponent)
	{
		AbilitySystemComponent = CreateDefaultSubobject<UNGAbilitySystemComponent>(TEXT("Ability System Component"));
	}
	
	if (!AttributeSet)
	{
		AttributeSet = CreateDefaultSubobject<UNGAttributeSet>(TEXT("AttributeSet"));
	}
	
	HPBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarComponent"));
	HPBarComponent->SetupAttachment(RootComponent);
	HPBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 10.f) + GetHalfCapsule());
	HPBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HPBarComponent->SetDrawSize(FVector2D(100.f, 20.f));
	
}

UAbilitySystemComponent* ANGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ANGCharacterBase::HandleGameplayCue(UObject* Self, FGameplayTag GameplayCueTag, EGameplayCueEvent::Type EventType,
	const FGameplayCueParameters& Parameters)
{
	IGameplayCueInterface::HandleGameplayCue(Self, GameplayCueTag, EventType, Parameters);
	
	if (GameplayCueTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Character.Hit"))))
	{
		if (EventType == EGameplayCueEvent::Executed)
		{
			PlayHitReaction();
		}
	}
}

void ANGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	LocationOffset = GetHalfCapsule();
	
	if (AbilitySystemComponent)
	{
		InitializeAttributes();
		
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UNGAttributeSet::GetHealthAttribute()).AddUObject(this, &ANGCharacterBase::OnHealthChanged);
	}
	
	UpdateHPBar();
}

void ANGCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANGCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	UpdateHPBar();
}

void ANGCharacterBase::OnAttackRangeChanged(const FOnAttributeChangeData& Data)
{
	
}

void ANGCharacterBase::PlayHitReaction()
{
	UE_LOG(LogTemp, Warning, TEXT("Hit Reaction Executed"));
	if (DamagedMontage)
	{
		if (const USkeletalMeshComponent* TempMesh = GetMesh())
		{
			if (UAnimInstance* AnimInstance = TempMesh->GetAnimInstance())
			{
				AnimInstance->Montage_Stop(.2f);
			}
		}
		
		PlayAnimMontage(DamagedMontage);
	}
}

void ANGCharacterBase::InitializeAttributes()
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

FVector ANGCharacterBase::GetHalfCapsule() const
{
	FVector CapsuleHalfHeight = FVector::ZeroVector;

	CapsuleHalfHeight.Z = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	return CapsuleHalfHeight;
}

UAnimMontage* ANGCharacterBase::GetAttackMontage() const
{
	return AttackMontage;
}

void ANGCharacterBase::Die()
{
	FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("State.Dead"));
	
	if (GetAbilitySystemComponent()->HasMatchingGameplayTag(DeadTag))	return;
	
	// AddLooseGameplayTag는 메모리 상에서만 일시적 태그를 붙일 때 유용
	GetAbilitySystemComponent()->AddLooseGameplayTag(DeadTag);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(true); //레그돌
	
	if (ANGCombatGameMode* GM = Cast<ANGCombatGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->CharacterDied(this);
	}
	
	SetLifeSpan(1.0f);
	
	UE_LOG(LogTemp, Log, TEXT("%s has died"), *GetName());
}

void ANGCharacterBase::UpdateHPBar()
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
