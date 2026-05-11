// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGPoolableComponent.h"


// Sets default values for this component's properties
UNGPoolableComponent::UNGPoolableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UNGPoolableComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//모든 클라이언트에게 복제할 변수를 등록 아래는 예시
	//DOREPLIFETIME(UNGPoolableComponent, 변수명);
}

void UNGPoolableComponent::SetPoolActive(bool bInActive)
{
	if (GetOwner()->HasAuthority())
	{
		bIsActiveWithPool = bInActive;
		OnRep_IsActiveWithPool();
	}
}


void UNGPoolableComponent::OnRep_IsActiveWithPool()
{	
	OnPoolStateChanged.Broadcast(bIsActiveWithPool);
	
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->SetActorHiddenInGame(!bIsActiveWithPool);
		Owner->SetActorEnableCollision(bIsActiveWithPool);
		Owner->SetActorTickEnabled(bIsActiveWithPool);
	}
}

// Called when the game starts
void UNGPoolableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UNGPoolableComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

