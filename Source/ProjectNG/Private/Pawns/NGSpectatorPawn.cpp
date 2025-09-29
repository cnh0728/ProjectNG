// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Pawns/NGSpectatorPawn.h"
#include "Camera/CameraComponent.h"


// Sets default values
ANGSpectatorPawn::ANGSpectatorPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
}

// Called when the game starts or when spawned
void ANGSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ANGSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANGSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

