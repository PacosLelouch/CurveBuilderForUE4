// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/


#include "CurveBuilderTestPlayerController.h"
#include "CGDemoCanvas2D.h"
#include "UObject/ConstructorHelpers.h"

ACurveBuilderTestPlayerController::ACurveBuilderTestPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ACurveBuilderTestPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MaxSamplePointsNum = FMath::CeilToInt((Canvas2D->CanvasBoxYZ.Max.Y - Canvas2D->CanvasBoxYZ.Min.Y) / SamplePointDT) + 1;
	//for (FOccluderVertexArray& VArray : Canvas2D->DisplayPoints) {
	//	VArray.Reserve(MaxSamplePointsNum);
	//}
	//for (FOccluderVertexArray& VArray : Canvas2D->DisplayLines) {
	//	VArray.Reserve(MaxSamplePointsNum);
	//}
}

void ACurveBuilderTestPlayerController::BindOnRightMouseButtonReleased()
{
	OnRightMouseButtonReleased.AddDynamic(this, &ACurveBuilderTestPlayerController::AddControlPoint);
}

void ACurveBuilderTestPlayerController::BindOnKey1Released()
{
	OnCtrlAndKey1Released.AddDynamic(this, &ACurveBuilderTestPlayerController::ChangeToPolynomialCurve);
}

void ACurveBuilderTestPlayerController::BindOnKey2Released()
{
	OnCtrlAndKey2Released.AddDynamic(this, &ACurveBuilderTestPlayerController::ChangeToBezierCurve);
}

void ACurveBuilderTestPlayerController::BindOnKey3Released()
{
	OnCtrlAndKey3Released.AddDynamic(this, &ACurveBuilderTestPlayerController::ChangeToRationalBezierCurve);
}

void ACurveBuilderTestPlayerController::BindOnKey4Released()
{
	// Do nothing.
}

void ACurveBuilderTestPlayerController::BindOnKey5Released()
{
	OnCtrlAndKey5Released.AddDynamic(this, &ACurveBuilderTestPlayerController::ChangeToAllCurves);
}

void ACurveBuilderTestPlayerController::BindOnKey0Released()
{
	OnCtrlAndKey0Released.AddDynamic(this, &ACurveBuilderTestPlayerController::ClearCanvasEvent);
}

void ACurveBuilderTestPlayerController::ChangeCurveType(ECurveType Type)
{
	CurveType = Type;
	ResampleCurve();
}

void ACurveBuilderTestPlayerController::ClearCanvas()
{
	UE_LOG(LogTemp, Warning, TEXT("Clear Canvas"));
	Canvas2D->DisplayPoints[0].Array.Empty(MaxSamplePointsNum);
	for (int32 Layer = 0; Layer < Curves.Num(); ++Layer) {
		const TPair<ECurveType, TSharedPtr<FPlanarCurve3> >& CurvePair = Curves[Layer];
		Canvas2D->DisplayLines[Layer].Array.Empty(MaxSamplePointsNum);
	}
	Canvas2D->ClearDrawing();
	ControlPoints.Empty(0);
	Curves.Empty(0);
}

void ACurveBuilderTestPlayerController::OnParamsInputChanged()
{
	ResampleCurve();
}

void ACurveBuilderTestPlayerController::AddControlPoint(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	float Distance = 0;
	FVector HitPoint(0, 0, 0);
	bool bHit = TraceToCanvas(Distance, HitPoint, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Right Mouse Button Released: %s, %s. %s"),
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString(), (bHit ? TEXT("true") : TEXT("false")));
	UE_LOG(LogTemp, Warning, TEXT("Hit Point: %s. Distance: %.3lf"),
		*HitPoint.ToCompactString(), Distance);
	if (!bHit) {
		return;
	}
	Canvas2D->DisplayPoints[0].Array.Add(HitPoint);
	ControlPoints.Add(FVector(Canvas2D->FromCanvasPoint(HitPoint) * ParamsInput.CurrentWeight, ParamsInput.CurrentWeight));
	if ((ControlPoints.Num() & 3) == 0) { // Num % 4 == 0
		FVector* Data = ControlPoints.GetData() + (ControlPoints.Num() - 4);
		Curves.Add(MakeTuple(ECurveType::Polynomial, TSharedPtr<FPlanarCurve3>(new FPlanarPolynomialCurve3(Data))));
		Curves.Add(MakeTuple(ECurveType::Bezier, TSharedPtr<FPlanarCurve3>(new FPlanarBezierCurve3(Data))));
		Curves.Add(MakeTuple(ECurveType::RationalBezier, TSharedPtr<FPlanarCurve3>(new FPlanarRationalBezierCurve3(Data))));
	}
	ResampleCurve();
}

void ACurveBuilderTestPlayerController::ChangeToPolynomialCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Polynomial Curve"));
	ChangeCurveType(ECurveType::Polynomial);
}

void ACurveBuilderTestPlayerController::ChangeToBezierCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Bezier Curve"));
	ChangeCurveType(ECurveType::Bezier);
}

void ACurveBuilderTestPlayerController::ChangeToRationalBezierCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Rational Bezier Curve"));
	ChangeCurveType(ECurveType::RationalBezier);
}

void ACurveBuilderTestPlayerController::ChangeToAllCurves(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("All Curves"));
	ChangeCurveType(ECurveType::PlaceHolder);
}

void ACurveBuilderTestPlayerController::ClearCanvasEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	ClearCanvas();
}

void ACurveBuilderTestPlayerController::ResampleCurve()
{
	Canvas2D->ClearDrawing();
	Canvas2D->DrawPoints();
	TSet<ECurveType> EnabledTypes;
	switch (CurveType) {
	case ECurveType::PlaceHolder:
	case ECurveType::Polynomial:
	{
		EnabledTypes.Add(ECurveType::Polynomial);
		if (CurveType != ECurveType::PlaceHolder) {
			break;
		}
	}
	case ECurveType::Bezier:
	{
		EnabledTypes.Add(ECurveType::Bezier);
		if (CurveType != ECurveType::PlaceHolder) {
			break;
		}
	}
	case ECurveType::RationalBezier:
	{
		EnabledTypes.Add(ECurveType::RationalBezier);
		if (CurveType != ECurveType::PlaceHolder) {
			break;
		}
	}
	}

	for (int32 Layer = 0; Layer < Curves.Num(); ++Layer) {
		TPair<ECurveType, TSharedPtr<FPlanarCurve3> >& CurvePair = Curves[Layer];
		if (!EnabledTypes.Contains(CurvePair.Key)) {
			continue;
		}
		TSharedPtr<FPlanarCurve3>& CurvePtr = CurvePair.Value;
		TArray<TTuple<double, double> > SampleResults;
		for (double T = 0.; T <= 1.; T += SamplePointDT) {
			auto Pos = CurvePtr->GetPosition(T);
			SampleResults.Add(MakeTuple((double)Pos[0], (double)Pos[1]));
		}
		Canvas2D->DisplayLines[Layer].Array.Empty(MaxSamplePointsNum);
		Canvas2DTo3D(Canvas2D->DisplayLines[Layer].Array, SampleResults, true);
		Canvas2D->DrawLines(Layer);
	}
}
