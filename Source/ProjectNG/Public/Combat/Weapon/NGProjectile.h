// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "NGProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PROJECTNG_API ANGProjectile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANGProjectile();

	void SetSpecHandle(const FGameplayEffectSpecHandle InSpecHandle) { SpecHandle = InSpecHandle; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	

	UPROPERTY(VisibleAnywhere)
	USphereComponent* SphereComponent;
	
	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	//어빌리티로부터 전달받은 데미지 정보
	FGameplayEffectSpecHandle SpecHandle;	
	
public:
	virtual void Tick(float DeltaTime) override;
};
