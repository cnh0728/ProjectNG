// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/NGUnitInfoWidget.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGPawnAttributeSet.h"
#include "Pawn/NGUnitPawn.h"

void UNGUnitInfoWidget::SetTargetUnit(ANGPawnBase* NewUnit)
{
	//기존 바인딩 해제
	ClearTargetUnit();
	
	if (!NewUnit)	return;
	
	UAbilitySystemComponent* ASC = NewUnit->GetAbilitySystemComponent();
	
	if (!ASC) return;
	
	TargetASC = Cast<UNGAbilitySystemComponent>(ASC);
	
	if (!TargetASC.IsValid())	return;
	
	float CurrentHP = ASC->GetNumericAttribute(UNGPawnAttributeSet::GetHealthAttribute());
	float MaxHP = ASC->GetNumericAttribute(UNGPawnAttributeSet::GetMaxHealthAttribute());
	float Strength = ASC->GetNumericAttribute(UNGPawnAttributeSet::GetStrengthAttribute());
	
	OnUpdateUnitInfo(CurrentHP, MaxHP, Strength);
	
	//체력이 바뀌었을때 호출되도록 구독해두기
	HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UNGPawnAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UNGUnitInfoWidget::OnHealthChanged);
	
	StrengthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UNGPawnAttributeSet::GetStrengthAttribute())
		.AddUObject(this, &UNGUnitInfoWidget::OnStrengthChanged);
	
}

void UNGUnitInfoWidget::ClearTargetUnit()
{
	if (TargetASC.IsValid())
	{
		TargetASC->GetGameplayAttributeValueChangeDelegate(UNGPawnAttributeSet::GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
		TargetASC->GetGameplayAttributeValueChangeDelegate(UNGPawnAttributeSet::GetStrengthAttribute()).Remove(StrengthChangedDelegateHandle);
	}
	
	TargetASC.Reset();
}

void UNGUnitInfoWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (TargetASC.IsValid())
	{
		float MaxHP = TargetASC->GetNumericAttribute(UNGPawnAttributeSet::GetMaxHealthAttribute());
		float Str = TargetASC->GetNumericAttribute(UNGPawnAttributeSet::GetStrengthAttribute());
		OnUpdateUnitInfo(Data.NewValue, MaxHP, Str);
	}
}

void UNGUnitInfoWidget::OnStrengthChanged(const FOnAttributeChangeData& Data)
{
	if (TargetASC.IsValid())
	{
		float HP = TargetASC->GetNumericAttribute(UNGPawnAttributeSet::GetHealthAttribute());
		float MaxHP = TargetASC->GetNumericAttribute(UNGPawnAttributeSet::GetMaxHealthAttribute());
		OnUpdateUnitInfo(HP, MaxHP, Data.NewValue);
	}
}