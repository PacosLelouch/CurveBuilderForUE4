// Fill out your copyright notice in the Description page of Project Settings.

#include "CurveControlWidget.h"


void UFittingControlWidget::BindValues()
{
	TextGaussianSigma->SetText(FText::FromString("Sigma"));
	TextRegressionSpan->SetText(FText::FromString("Span"));
	TextRidgeLambda->SetText(FText::FromString("Lambda"));
	TextPF->SetText(GetTextPF());
	TextGF->SetText(GetTextGF());
	TextPR->SetText(GetTextPR());
	TextRR->SetText(GetTextRR());
	TextAll->SetText(GetTextAll());
	TextCurrentMethod->SetText(GetTextCurrentMethod());
	SpinBoxSigma->ValueDelegate.BindDynamic(this, &UFittingControlWidget::GetSpinBoxSigma);
	SpinBoxSpan->ValueDelegate.BindDynamic(this, &UFittingControlWidget::GetSpinBoxSpan);
	SpinBoxLambda->ValueDelegate.BindDynamic(this, &UFittingControlWidget::GetSpinBoxLambda);

	SpinBoxSigma->SetMinValue(0.01);
	SpinBoxSigma->SetMinSliderValue(0.01);
	SpinBoxSigma->SetMaxSliderValue(50.0);
	SpinBoxSigma->Delta = 0.01;
	SpinBoxSigma->SliderExponent = 2;

	SpinBoxSpan->SetMinValue(0.0);
	SpinBoxSpan->SetMinSliderValue(0.0);
	SpinBoxSpan->SetMaxSliderValue(50.0);
	SpinBoxSpan->Delta = 1.0;
	SpinBoxSpan->SliderExponent = 1;

	SpinBoxLambda->SetMinValue(0.0);
	SpinBoxLambda->SetMinSliderValue(0.0);
	SpinBoxLambda->SetMaxSliderValue(50.0);
	SpinBoxSigma->Delta = 0.01;
	SpinBoxSigma->SliderExponent = 2;
}

void UFittingControlWidget::BindEvents()
{
	SpinBoxSigma->OnValueCommitted.AddDynamic(this, &UFittingControlWidget::CommitSpinBoxSigma);
	SpinBoxSpan->OnValueCommitted.AddDynamic(this, &UFittingControlWidget::CommitSpinBoxSpan);
	SpinBoxLambda->OnValueCommitted.AddDynamic(this, &UFittingControlWidget::CommitSpinBoxLambda);

	//ButtonAll->OnReleased.AddDynamic(this, &UFittingControlWidget::ReleaseButtonAll);
	ButtonAll->SetIsEnabled(false);
	ButtonPF->OnReleased.AddDynamic(this, &UFittingControlWidget::ReleaseButtonPF);
	ButtonGF->OnReleased.AddDynamic(this, &UFittingControlWidget::ReleaseButtonGF);
	ButtonPR->OnReleased.AddDynamic(this, &UFittingControlWidget::ReleaseButtonPR);
	ButtonRR->OnReleased.AddDynamic(this, &UFittingControlWidget::ReleaseButtonRR);
	ButtonClear->OnReleased.AddDynamic(this, &UFittingControlWidget::ReleaseButtonClear);
}

FText UFittingControlWidget::GetTextCurrentMethod()
{
	//if (Controller) {
	//	switch (Controller->FittingMethod) {
	//	case EFunctionFittingMethod::PolynomialFitting:
	//		return GetTextPF();
	//	case EFunctionFittingMethod::GaussianFitting:
	//		return GetTextGF();
	//	case EFunctionFittingMethod::PolynomialRegression:
	//		return GetTextPR();
	//	case EFunctionFittingMethod::RidgeRegression:
	//		return GetTextRR();
	//	case EFunctionFittingMethod::All:
	//		return GetTextAll();
	//	}
	//}
	static FText NoneText = FText::FromString("None");
	return NoneText;
}

FText UFittingControlWidget::GetTextPF()
{
	static FText Text = FText::FromString("Polynomial Fitting");
	return Text;
}

FText UFittingControlWidget::GetTextGF()
{
	static FText Text = FText::FromString("Gaussian Fitting");
	return Text;
}

FText UFittingControlWidget::GetTextPR()
{
	static FText Text = FText::FromString("Polynomial Regression");
	return Text;
}

FText UFittingControlWidget::GetTextRR()
{
	static FText Text = FText::FromString("Ridge Regression");
	return Text;
}

FText UFittingControlWidget::GetTextAll()
{
	static FText Text = FText::FromString("All");
	return Text;
}

float UFittingControlWidget::GetSpinBoxSigma()
{
	//if (Controller) {
	//	return Controller->ParamsInput.GaussianSigma;
	//}
	return SpinBoxSigma->GetMinValue();
}

float UFittingControlWidget::GetSpinBoxSpan()
{
	//if (Controller) {
	//	return static_cast<float>(Controller->ParamsInput.RegressionSpan);
	//}
	return SpinBoxSpan->GetMinValue();
}

float UFittingControlWidget::GetSpinBoxLambda()
{
	//if (Controller) {
	//	return Controller->ParamsInput.RidgeLambda;
	//}
	return SpinBoxLambda->GetMinValue();
}

void UFittingControlWidget::CommitSpinBoxSigma(float InValue, ETextCommit::Type CommitMethod)
{
	//if (Controller) {
	//	Controller->ParamsInput.GaussianSigma = InValue;
	//	Controller->OnParamsInputChanged();
	//}
}

void UFittingControlWidget::CommitSpinBoxSpan(float InValue, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::Type::Default) {
		SpinBoxSpan->SetValue(FMath::RoundToFloat(InValue));
	}
	//if (Controller) {
	//	Controller->ParamsInput.RegressionSpan = FMath::RoundToInt(InValue);
	//	Controller->OnParamsInputChanged();
	//}
}

void UFittingControlWidget::CommitSpinBoxLambda(float InValue, ETextCommit::Type CommitMethod)
{
	//if (Controller) {
	//	Controller->ParamsInput.RidgeLambda = InValue;
	//	Controller->OnParamsInputChanged();
	//}
}

void UFittingControlWidget::ReleaseButtonAll()
{
	//if (Controller) {
	//	Controller->ChangeFittingMethod(EFunctionFittingMethod::All);
	//}
	TextCurrentMethod->SetText(GetTextCurrentMethod());
}

void UFittingControlWidget::ReleaseButtonPF()
{
	//if (Controller) {
	//	Controller->ChangeFittingMethod(EFunctionFittingMethod::PolynomialFitting);
	//}
	TextCurrentMethod->SetText(GetTextCurrentMethod());
}

void UFittingControlWidget::ReleaseButtonGF()
{
	//if (Controller) {
	//	Controller->ChangeFittingMethod(EFunctionFittingMethod::GaussianFitting);
	//}
	TextCurrentMethod->SetText(GetTextCurrentMethod());
}

void UFittingControlWidget::ReleaseButtonPR()
{
	//if (Controller) {
	//	Controller->ChangeFittingMethod(EFunctionFittingMethod::PolynomialRegression);
	//}
	TextCurrentMethod->SetText(GetTextCurrentMethod());
}

void UFittingControlWidget::ReleaseButtonRR()
{
	//if (Controller) {
	//	Controller->ChangeFittingMethod(EFunctionFittingMethod::RidgeRegression);
	//}
	TextCurrentMethod->SetText(GetTextCurrentMethod());
}

void UFittingControlWidget::ReleaseButtonClear()
{
	if (Controller) {
		Controller->ClearCanvas();
	}
}
