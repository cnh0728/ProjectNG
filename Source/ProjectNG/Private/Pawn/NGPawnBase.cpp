// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Pawn/NGPawnBase.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/NGPoolableComponent.h"
#include "Game/NGUnitDataManager.h"
#include "GameModes/NGInGameGameMode.h"
#include "UI/NGWidgetInterface.h"

ANGPawnBase::ANGPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;		//네트워크 복제 활성화
	SetReplicatingMovement(true);	//위치 속도 복제 활성화
	
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

void ANGPawnBase::BeginPlay()
{
	Super::BeginPlay();
	
	ensureMsgf(IdentificationTag.IsValid(), TEXT("UnitIdTag is not valid for %s"), *GetName());
	
	LocationOffset = GetHalfCapsule();
	
	if (AbilitySystemComponent)
	{
		InitializeAttributes();
		
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UNGAttributeSet::GetHealthAttribute()).AddUObject(this, &ANGPawnBase::OnHealthChanged);
	}
	
	UpdateHPBar();
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

void ANGPawnBase::Activate()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	
	UNGUnitDataManager* UnitDataManager = GetWorld()->GetGameInstance()->GetSubsystem<UNGUnitDataManager>();
	if (!UnitDataManager) return;
	
	const FUnitAbilityData* UnitData = UnitDataManager->GetUnitAbilityData(IdentificationTag);
	if (UnitData)
	{
		InitAbilityData(*UnitData);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 데이터 테이블에서 태그(%s)를 찾을 수 없습니다!"), *GetName(), *IdentificationTag.ToString());
	}
}

void ANGPawnBase::Deactivate()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void ANGPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
		// AbilitySystemComponent->InitStats(UNGAttributeSet::StaticClass(), DefaultAttributeTable);
	}
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
	
	// Todo: 삭제가 되는 것이 아니라, 비활성화해서 pool로 복귀
	Deactivate();
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(true); //레그돌
	
	if (ANGInGameGameMode* GM = GetWorld()->GetAuthGameMode<ANGInGameGameMode>())
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

void ANGPawnBase::InitAbilityData(const FUnitAbilityData& AbilityData)
{
	if (!AbilitySystemComponent || !AttributeSet) return;
	
	if (HasAuthority())
	{
		// Setup Tag
		IdentificationTag = AbilityData.IdentificationTag;
		AbilitySystemComponent->AddLooseGameplayTags(AbilityData.OwnedTags);
		
		// Initialize Stat
		for (const auto& Stat : AbilityData.DefaultStats)
		{
			AbilitySystemComponent->SetNumericAttributeBase(Stat.Key, Stat.Value);
		}
	}
}
