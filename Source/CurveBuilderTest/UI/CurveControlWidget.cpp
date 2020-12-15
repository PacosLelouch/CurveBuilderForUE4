// Fill out your copyright notice in the Description page of Project Settings.

#include "CurveControlWidget.h"

#define GetValueRef GetValue().Get

void UCurveControlWidget::BindValues()
{
	Text_Remake->SetText(FText::FromString("Remake"));
	Text_Clear->SetText(FText::FromString("Clear"));
	Text_PosPrefix->SetText(FText::FromString("Pos"));
	Text_Pos->SetText(GetPos());
	Text_ParamPrefix->SetText(FText::FromString("Parameter"));
	Text_Param->SetText(GetParam());
	Text_CurrentContinuity->SetText(FText::FromString("Current Continuity"));
	Text_NextPointContinuity->SetText(FText::FromString("Next Point Continuity"));

	ComboBoxString_CurrentContinuity->ClearOptions();
	ComboBoxString_CurrentContinuity->AddOption("C0");
	ComboBoxString_CurrentContinuity->AddOption("G1");
	ComboBoxString_CurrentContinuity->AddOption("C1");
	ComboBoxString_CurrentContinuity->AddOption("G2");
	ComboBoxString_CurrentContinuity->AddOption("C2");
	ComboBoxString_CurrentContinuity->AddOption("?");
	ComboBoxString_CurrentContinuity->SetSelectedIndex(ComboBoxString_CurrentContinuity->GetOptionCount() - 1);
	ComboBoxString_CurrentContinuity->SetIsEnabled(false);

	ComboBoxString_NextPointContinuity->ClearOptions();
	ComboBoxString_NextPointContinuity->AddOption("C0");
	ComboBoxString_NextPointContinuity->AddOption("G1");
	ComboBoxString_NextPointContinuity->AddOption("C1");
	ComboBoxString_NextPointContinuity->AddOption("G2");
	ComboBoxString_NextPointContinuity->AddOption("C2");
	ComboBoxString_NextPointContinuity->SetSelectedIndex(3);

	//TextGaussianSigma->SetText(FText::FromString("Sigma"));
	//TextRegressionSpan->SetText(FText::FromString("Span"));
	//TextRidgeLambda->SetText(FText::FromString("Lambda"));
	//TextPF->SetText(GetTextPF());
	//TextGF->SetText(GetTextGF());
	//TextPR->SetText(GetTextPR());
	//TextRR->SetText(GetTextRR());
	//TextAll->SetText(GetTextAll());
	//TextCurrentMethod->SetText(GetTextCurrentMethod());
	//SpinBoxSigma->ValueDelegate.BindDynamic(this, &UCurveControlWidget::GetSpinBoxSigma);
	//SpinBoxSpan->ValueDelegate.BindDynamic(this, &UCurveControlWidget::GetSpinBoxSpan);
	//SpinBoxLambda->ValueDelegate.BindDynamic(this, &UCurveControlWidget::GetSpinBoxLambda);

	//SpinBoxSigma->SetMinValue(0.01);
	//SpinBoxSigma->SetMinSliderValue(0.01);
	//SpinBoxSigma->SetMaxSliderValue(50.0);
	//SpinBoxSigma->Delta = 0.01;
	//SpinBoxSigma->SliderExponent = 2;

	//SpinBoxSpan->SetMinValue(0.0);
	//SpinBoxSpan->SetMinSliderValue(0.0);
	//SpinBoxSpan->SetMaxSliderValue(50.0);
	//SpinBoxSpan->Delta = 1.0;
	//SpinBoxSpan->SliderExponent = 1;

	//SpinBoxLambda->SetMinValue(0.0);
	//SpinBoxLambda->SetMinSliderValue(0.0);
	//SpinBoxLambda->SetMaxSliderValue(50.0);
	//SpinBoxSigma->Delta = 0.01;
	//SpinBoxSigma->SliderExponent = 2;
}

void UCurveControlWidget::BindEvents()
{
	//SpinBoxSigma->OnValueCommitted.AddDynamic(this, &UCurveControlWidget::CommitSpinBoxSigma);
	//SpinBoxSpan->OnValueCommitted.AddDynamic(this, &UCurveControlWidget::CommitSpinBoxSpan);
	//SpinBoxLambda->OnValueCommitted.AddDynamic(this, &UCurveControlWidget::CommitSpinBoxLambda);

	Button_Remake->OnReleased.AddDynamic(this, &UCurveControlWidget::ReleaseButtonRemake);
	Button_Clear->OnReleased.AddDynamic(this, &UCurveControlWidget::ReleaseButtonClear);
	ComboBoxString_CurrentContinuity->OnSelectionChanged.AddDynamic(this, &UCurveControlWidget::SelectCurrentContinuity);
	ComboBoxString_NextPointContinuity->OnSelectionChanged.AddDynamic(this, &UCurveControlWidget::SelectNextPointContinuity);
}

void UCurveControlWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Text_Pos->SetText(GetPos());
	Text_Param->SetText(GetParam());
	UpdateCurrentContinuity();
}

FText UCurveControlWidget::GetPos()
{
	if (Controller) {
		if (Controller->SelectedNode) {
			FVector4* PHPtr = nullptr;
			switch (Controller->HoldingPointType.Get(ESelectedNodeCtrlPointType::Current)) {
			case ESelectedNodeCtrlPointType::Current:
				PHPtr = &Controller->SelectedNode->GetValueRef().Pos;
				return FText::FromString(FString::Printf(TEXT("(%.3lf, %.3lf)"), PHPtr->X, PHPtr->Y));
			case ESelectedNodeCtrlPointType::Previous:
				PHPtr = &Controller->SelectedNode->GetValueRef().PrevCtrlPointPos;
				return FText::FromString(FString::Printf(TEXT("(%.3lf, %.3lf)"), PHPtr->X, PHPtr->Y));
			case ESelectedNodeCtrlPointType::Next:
				PHPtr = &Controller->SelectedNode->GetValueRef().NextCtrlPointPos;
				return FText::FromString(FString::Printf(TEXT("(%.3lf, %.3lf)"), PHPtr->X, PHPtr->Y));
			}
		}
	}
	return FText::FromString("?");
}

FText UCurveControlWidget::GetParam()
{
	if (Controller) {
		if (Controller->SelectedNode) {
			return FText::FromString(FString::Printf(TEXT("%lf"), Controller->SelectedNode->GetValueRef().Param));
		}
	}
	return FText::FromString("?");
}

void UCurveControlWidget::UpdateCurrentContinuity()
{
	if (Controller) {
		if (Controller->SelectedNode) {
			ComboBoxString_CurrentContinuity->SetIsEnabled(true);
			ComboBoxString_CurrentContinuity->SetSelectedIndex(static_cast<int32>(Controller->SelectedNode->GetValueRef().Continuity));
		}
		else {
			ComboBoxString_CurrentContinuity->SetIsEnabled(false);
			ComboBoxString_CurrentContinuity->SetSelectedIndex(ComboBoxString_CurrentContinuity->GetOptionCount() - 1);
		}
	}
}

void UCurveControlWidget::ReleaseButtonRemake()
{
	if (Controller) {
		Controller->RemakeBezierC2();
	}
}

void UCurveControlWidget::ReleaseButtonClear()
{
	if (Controller) {
		Controller->ClearCanvas();
	}
}

void UCurveControlWidget::SelectCurrentContinuity(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	int32 Index = ComboBoxString_CurrentContinuity->FindOptionIndex(SelectedItem);
	if (Controller && Index >= 0 && Index < ComboBoxString_CurrentContinuity->GetOptionCount() - 1) {
		Controller->SelectedNode->GetValueRef().Continuity = (EEndPointContinuity)Index;
	}
}

void UCurveControlWidget::SelectNextPointContinuity(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	int32 Index = ComboBoxString_NextPointContinuity->FindOptionIndex(SelectedItem);
	if (Controller && Index >= 0) {
		Controller->NewPointContinuityInit = (EEndPointContinuity)Index;
	}
}

#undef GetValueRef
