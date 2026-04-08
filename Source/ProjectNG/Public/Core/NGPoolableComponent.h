// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NGPoolableComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTNG_API UNGPoolableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UNGPoolableComponent();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetPoolActive(bool bInActive);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPoolStateChanged, bool, bNewActive);
	
	UPROPERTY(BlueprintAssignable, Category = "Pool")
	FOnPoolStateChanged OnPoolStateChanged;
	
protected:
	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActiveWithPool;
	
	UFUNCTION()
	void OnRep_IsActiveWithPool();
	
	// Called when the game starts
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
