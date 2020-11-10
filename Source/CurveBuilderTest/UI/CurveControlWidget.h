// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ControlWidget.h"
#include "CurveControlWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UFittingControlWidget : public UControlWidget
{
	GENERATED_BODY()
public:

	virtual void BindValues() override;

	virtual void BindEvents() override;

public:
	UFUNCTION(BlueprintGetter)
	FText GetTextCurrentMethod();

	UFUNCTION(BlueprintGetter)
	FText GetTextPF();

	UFUNCTION(BlueprintGetter)
	FText GetTextGF();

	UFUNCTION(BlueprintGetter)
	FText GetTextPR();

	UFUNCTION(BlueprintGetter)
	FText GetTextRR();

	UFUNCTION(BlueprintGetter)
	FText GetTextAll();

	UFUNCTION(BlueprintGetter)
	float GetSpinBoxSigma();

	UFUNCTION(BlueprintGetter)
	float GetSpinBoxSpan();

	UFUNCTION(BlueprintGetter)
	float GetSpinBoxLambda();

public:
	UFUNCTION(BlueprintCallable)
	void CommitSpinBoxSigma(float InValue, ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintCallable)
	void CommitSpinBoxSpan(float InValue, ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintCallable)
	void CommitSpinBoxLambda(float InValue, ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonAll();

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonPF();

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonGF();

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonPR();

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonRR();

	UFUNCTION(BlueprintCallable)
	void ReleaseButtonClear();

public:

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* ButtonAll;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* ButtonClear;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* ButtonPF;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* ButtonGF;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* ButtonPR;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Button", Meta = (BindWidget))
	UButton* ButtonRR;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|SpinBox", Meta = (BindWidget))
	USpinBox* SpinBoxLambda;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|SpinBox", Meta = (BindWidget))
	USpinBox* SpinBoxSigma;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|SpinBox", Meta = (BindWidget))
	USpinBox* SpinBoxSpan;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextCurrentMethod;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextClear;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextAll;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextPF;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextGF;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextPR;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextRR;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextGaussianSigma;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextRegressionSpan;

	UPROPERTY(BlueprintReadOnly, Category = "Widget|Text", Meta = (BindWidget))
	UTextBlock* TextRidgeLambda;
};