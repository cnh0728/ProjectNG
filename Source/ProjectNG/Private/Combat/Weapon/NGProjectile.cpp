// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Combat/Weapon/NGProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/NGCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Core/NGPoolSubSystem.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ANGProjectile::ANGProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComponent;
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));
	
	// ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// ProjectileMovementComponent->InitialSpeed = 2000.f;
	// ProjectileMovementComponent->MaxSpeed = 2000.f;
	// ProjectileMovementComponent->bRotationFollowsVelocity = true; // 날아가는 방향으로 고개 돌리기
}

// Called when the game starts or when spawned
void ANGProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ANGProjectile::OnProjectileOverlap);
}

void ANGProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && !OtherActor->IsA<ANGProjectile>())
	{
		check(Target);
		UE_LOG(LogTemp, Log, TEXT("Other Actor Enter, OtherActorName = %s, TargetName = %s"), *OtherActor->GetName(), *Target->GetName());

		if (OtherActor == Target)
		{
			UE_LOG(LogTemp, Log, TEXT("Actor is Target"));
			//이거 타겟이랑 같은지 체크
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
			if (TargetASC && SpecHandle.IsValid())
			{
				ReleaseProjectile();
				
				TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void ANGProjectile::ReleaseProjectile()
{
	UNGPoolSubSystem* Pool = GetWorld()->GetSubsystem<UNGPoolSubSystem>();
	if (Pool)
	{
		Pool->ReleaseSegment(this);
	}
}

void ANGProjectile::SetTarget(ANGCharacterBase* NewTarget)
{
	Target = NewTarget;
	// ProjectileMovementComponent->HomingTargetComponent = NewTarget->GetRootComponent();
}

// Called every frame
void ANGProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (Target)
	{		
		FVector CurrentLocation = GetActorLocation();
		FVector TargetLocation = Target->GetActorLocation();
		FVector DesiredDirection = (TargetLocation - CurrentLocation).GetSafeNormal();
		
		FRotator CurrentRotation = GetActorRotation();
		FRotator TargetRotation = Target->GetActorRotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotateSpeed);
		
		SetActorRotation(NewRotation);
		
		//Projectile랑 유동적으로 바꿔가면서 쓸거면 여기서 분기처리
		AddActorWorldOffset(DesiredDirection * MoveSpeed * DeltaTime, true);
	}
	
	//Target이 없으면 어짜피 if문 들어가고, 있으면 IsDead까지 실행
	if (!Target || Target->IsDead())
	{
		//TODO: 바로 없애지말고 죽기전 타겟위치까지는 가게하는게 나아보임 -> 매틱 타겟 위치 기록해놓고 타겟없어지면 거기로 가면 될듯
		ReleaseProjectile();
	}
}

