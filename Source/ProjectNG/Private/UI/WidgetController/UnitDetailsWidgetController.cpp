// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/WidgetController/UnitDetailsWidgetController.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Data/UnitAttributeInfo.h"
#include "Pawn/NGUnitPawn.h"

void UUnitDetailsWidgetController::BroadcastInitialValues()
{
	if (!TargetUnit.IsValid() || AttributeInfo == nullptr) return;
	
	UAbilitySystemComponent* ASC = TargetUnit->GetAbilitySystemComponent();
	if (!ASC) return;
	

	for (const FUnitAttributeInfo& Info : AttributeInfo->AttributeInformation)
	{
		float CurrentValue = ASC->GetNumericAttribute(Info.Attribute);
		OnAttributeChangedDelegate.Broadcast(Info.AttributeTag, CurrentValue);
	}
}

void UUnitDetailsWidgetController::BindCallbacksToDependencies()
{
}

void UUnitDetailsWidgetController::SetTargetUnit(ANGUnitPawn* NewUnit)
{
	ClearTargetUnit();
	
	TargetUnit = NewUnit;
	
	if (TargetUnit.IsValid() && AttributeInfo != nullptr)
	{
		UAbilitySystemComponent* ASC = TargetUnit->GetAbilitySystemComponent();
		
		for (const FUnitAttributeInfo& Info : AttributeInfo->AttributeInformation)
		{
			if (!Info.Attribute.IsValid()) continue;
			
			FDelegateHandle Handle = ASC->GetGameplayAttributeValueChangeDelegate(Info.Attribute)
				.AddUObject(this, &ThisClass::OnAnyAttributeChanged);
            
			AttributeDelegateHandles.Add(Info.Attribute, Handle);
		}
		
		BroadcastInitialValues();
		
		OnUnitVisibilityChangedDelegate.Broadcast(true);
	}
	else
	{
		OnUnitVisibilityChangedDelegate.Broadcast(false);
	}
}

void UUnitDetailsWidgetController::ClearTargetUnit()
{
	if (TargetUnit.IsValid() && TargetUnit->GetAbilitySystemComponent())
	{
		for (const auto& HandlePair : AttributeDelegateHandles)
		{
			TargetUnit->GetAbilitySystemComponent()->GetGameplayAttributeValueChangeDelegate(HandlePair.Key).Remove(HandlePair.Value);
		}
	}
	
	AttributeDelegateHandles.Empty();
	TargetUnit.Reset();
}

void UUnitDetailsWidgetController::OnAnyAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 값이 변하면, 변경된 어트리뷰트가 데이터 에셋의 누구인지 다시 찾아서 태그와 함께 브로드캐스트
	if (AttributeInfo)
	{
		for (const FUnitAttributeInfo& Info : AttributeInfo->AttributeInformation)
		{
			if (Info.Attribute == Data.Attribute)
			{
				// UI 블루프린트 쪽으로 태그와 새로운 값을 발사!
				OnAttributeChangedDelegate.Broadcast(Info.AttributeTag, Data.NewValue);
				break;
			}
		}
	}
}
