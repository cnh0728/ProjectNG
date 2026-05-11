// Copyright (c) 2025 TeamNG. All Rights Reserved.


#include "UI/NGUnitInfoWidget.h"

#include "AbilitySystem/NGAbilitySystemComponent.h"
#include "AbilitySystem/NGAttributeSet.h"
#include "Pawn/NGUnitPawn.h"

void UNGUnitInfoWidget::SetTargetUnit(ANGUnitPawn* NewUnit)
{
	//기존 바인딩 해제
	ClearTargetUnit();
	
	if (!NewUnit)	return;
	
	UAbilitySystemComponent* ASC = NewUnit->GetAbilitySystemComponent();
	
	if (!ASC) return;
	
	TargetASC = Cast<UNGAbilitySystemComponent>(ASC);
	
	if (!TargetASC.IsValid())	return;
	
	float CurrentHP = ASC->GetNumericAttribute(UNGAttributeSet::GetHealthAttribute());
	float MaxHP = ASC->GetNumericAttribute(UNGAttributeSet::GetMaxHealthAttribute());
	float Strength = ASC->GetNumericAttribute(UNGAttributeSet::GetStrengthAttribute());
	
	OnUpdateUnitInfo(CurrentHP, MaxHP, Strength);
	
	//체력이 바뀌었을때 호출되도록 구독해두기
	HealthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UNGAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UNGUnitInfoWidget::OnHealthChanged);
	
	StrengthChangedDelegateHandle = ASC->GetGameplayAttributeValueChangeDelegate(UNGAttributeSet::GetStrengthAttribute())
		.AddUObject(this, &UNGUnitInfoWidget::OnStrengthChanged);
	
}

void UNGUnitInfoWidget::ClearTargetUnit()
{
	if (TargetASC.IsValid())
	{
		TargetASC->GetGameplayAttributeValueChangeDelegate(UNGAttributeSet::GetHealthAttribute()).Remove(HealthChangedDelegateHandle);
		TargetASC->GetGameplayAttributeValueChangeDelegate(UNGAttributeSet::GetStrengthAttribute()).Remove(StrengthChangedDelegateHandle);
	}
	
	TargetASC.Reset();
}

void UNGUnitInfoWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	if (TargetASC.IsValid())
	{
		float MaxHP = TargetASC->GetNumericAttribute(UNGAttributeSet::GetMaxHealthAttribute());
		float Str = TargetASC->GetNumericAttribute(UNGAttributeSet::GetStrengthAttribute());
		OnUpdateUnitInfo(Data.NewValue, MaxHP, Str);
	}
}

void UNGUnitInfoWidget::OnStrengthChanged(const FOnAttributeChangeData& Data)
{
	if (TargetASC.IsValid())
	{
		float HP = TargetASC->GetNumericAttribute(UNGAttributeSet::GetHealthAttribute());
		float MaxHP = TargetASC->GetNumericAttribute(UNGAttributeSet::GetMaxHealthAttribute());
		OnUpdateUnitInfo(HP, MaxHP, Data.NewValue);
	}
}