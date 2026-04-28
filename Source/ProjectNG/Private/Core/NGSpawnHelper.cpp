// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "Core/NGSpawnHelper.h"

#include "Components/CapsuleComponent.h"
#include "Core/NGPoolSubSystem.h"
#include "ProjectNG/ProjectNG.h"

ANGPawnBase* UNGSpawnHelper::Internal_SpawnPawn(UObject* WorldContextObject, ::TSubclassOf<ANGPawnBase> PawnClass,
                                       FTransform SpawnTransform,
                                       AActor* Owner)
{
	if (!Owner->HasAuthority())	return nullptr;
	//여기서부터 아래가 소환로직
	
	UNGPoolSubSystem* Pool = WorldContextObject->GetWorld()->GetSubsystem<UNGPoolSubSystem>();
	if (!Pool)	return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	/// 캐릭터 스폰 오프셋이 발끝기준으로 소환되게 조정
	if (const ANGPawnBase* DefaultPawn = PawnClass.GetDefaultObject())
	{
		if (const UCapsuleComponent* Capsule = DefaultPawn->GetCapsuleComponent())
		{
			float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			SpawnTransform.AddToTranslation(FVector(0.f, 0.f, HalfHeight));
		}
	}
	
	ANGPawnBase* NewPawn = Pool->AcquirePawn(PawnClass, SpawnTransform, SpawnParams);
	if (!NewPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("NewPawn is nullptr"));
		return nullptr;
	}

	if (UCapsuleComponent* Capsule = NewPawn->GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_SelectableUnit, ECR_Block);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
	return NewPawn;
}
