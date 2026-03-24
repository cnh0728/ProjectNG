// Copyright (c) 2025 TeamNG. All Rights Reserved.

#include "Combat/Weapon/NGProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/NGCharacterBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


// Sets default values
ANGProjectile::ANGProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	// PrimaryActorTick.bCanEverTick = true;
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComponent;
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->MaxSpeed = 2000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 날아가는 방향으로 고개 돌리기
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
	if (OtherActor && OtherActor == Target)
	{
		//이거 타겟이랑 같은지 체크
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC && SpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			Destroy();
		}
	}
}

void ANGProjectile::SetTarget(ANGCharacterBase* NewTarget)
{
	Target = NewTarget;
	ProjectileMovementComponent->HomingTargetComponent = NewTarget->GetRootComponent();
}

// Called every frame
void ANGProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

