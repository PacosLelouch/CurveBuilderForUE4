// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ControlWidget.h"
#include "Components/ComboBoxString.h"
#include "CurveControlWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UCurveControlWidget : public UControlWidget
{
	GENERATED_BODY()
public:

	virtual void BindValues() override;

	virtual void BindEvents() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void UpdateCurrentContinuity();

public:

	UFUNCTION(BlueprintGetter)
	FText GetPos();

	UFUNCTION(BlueprintGetter)
	FText GetParam();

public:

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonRemake();

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonClear();

	UFUNCTION(BlueprintCallable)
	void SelectCurrentContinuity(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION(BlueprintCallable)
	void SelectNextPointContinuity(FString SelectedItem, ESelectInfo::Type SelectionType);

public:

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* Button_Remake;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_Remake;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* Button_Clear;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_Clear;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_PosPrefix;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_Pos;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_ParamPrefix;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_Param;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_CurrentContinuity;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Input", Meta = (BindWidget))
	UComboBoxString* ComboBoxString_CurrentContinuity;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* Text_NextPointContinuity;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Input", Meta = (BindWidget))
	UComboBoxString* ComboBoxString_NextPointContinuity;

};