// Copyright (c) 2025 TeamNG. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/NGPlayerState.h"
#include "ArenaManager.generated.h"

class AArena;

UENUM()
enum class EPossessArenaIdentification
{
	None,
	Home,
	Away,
};

USTRUCT()
struct FArenaAddress
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY()
	TObjectPtr<AArena> Arena;
	
	UPROPERTY()
	EPossessArenaIdentification PossessArenaIdentification;
};

UCLASS()
class PROJECTNG_API AArenaManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AArenaManager();

	void PossessArena(const FArenaAddress& NewArenaAddress);
	
	//아레나 옮기는 거 - 대기유닛 넘어가고 Combat넘어가고 같이 처리 그리고 PossessArena에서 카메라랑 이 함수 처리
	void MigrationUnits(const FArenaAddress& NewAddress) const;
	
	void PossessSpecPawn(const FArenaAddress& ArenaAddress) const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess=true))
	FArenaAddress PossessArenaAddress;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
