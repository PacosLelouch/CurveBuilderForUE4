// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/


#include "SplineTestPlayerController.h"
#include "CGDemoCanvas2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Logging/LogMacros.h"
#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSplineCtrl, Warning, All);

DEFINE_LOG_CATEGORY(LogSplineCtrl);

ASplineTestPlayerController::ASplineTestPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ASplineTestPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MaxSamplePointsNum = FMath::CeilToInt((Canvas2D->CanvasBoxYZ.Max.Y - Canvas2D->CanvasBoxYZ.Min.Y) / SamplePointDT) + 1;
	Splines.Empty();
	Splines.AddDefaulted();
}

void ASplineTestPlayerController::BindOnRightMouseButtonReleased()
{
	OnRightMouseButtonReleased.AddDynamic(this, &ASplineTestPlayerController::AddControlPointEvent);
}

void ASplineTestPlayerController::BindOnCtrlAndKey1Released()
{
	//OnCtrlAndKey1Released.AddDynamic(this, &ASplineTestPlayerController::ChangeConcatTypeToPoint);
	OnCtrlAndKey1Released.AddDynamic(this, &ASplineTestPlayerController::FlipDisplayControlPointEvent);
}

void ASplineTestPlayerController::BindOnCtrlAndKey2Released()
{
	//OnCtrlAndKey2Released.AddDynamic(this, &ASplineTestPlayerController::ChangeConcatTypeToCurve);
	OnCtrlAndKey2Released.AddDynamic(this, &ASplineTestPlayerController::FlipDisplaySmallTangentOfInternalKnotEvent);
}

void ASplineTestPlayerController::BindOnCtrlAndKey3Released()
{
	// Do nothing.
	OnCtrlAndKey3Released.AddDynamic(this, &ASplineTestPlayerController::FlipDisplaySmallCurvatureOfInternalKnotEvent);
}

void ASplineTestPlayerController::BindOnCtrlAndKey4Released()
{
	// Do nothing.
}

void ASplineTestPlayerController::BindOnCtrlAndKey5Released()
{
	// Do nothing.
	OnCtrlAndKey5Released.AddDynamic(this, &ASplineTestPlayerController::FlipConvertToPolynomialFormEvent);
}

void ASplineTestPlayerController::BindOnCtrlAndKey0Released()
{
	//Super::BindOnCtrlAndKey0Released();
	OnCtrlAndKey0Released.AddDynamic(this, &ASplineTestPlayerController::ClearCanvasEvent);
}

void ASplineTestPlayerController::BindOnEnterReleased()
{
	Super::BindOnEnterReleased();
	OnEnterReleased.AddDynamic(this, &ASplineTestPlayerController::AddNewSplineEvent);
}

void ASplineTestPlayerController::ChangeConcatType(ESplineConcatType Type)
{
	ConcatType = Type;
	ClearCanvas();
}

void ASplineTestPlayerController::FlipConvertToPolynomialForm()
{
	bConvertToPolynomialForm = !bConvertToPolynomialForm;
	UE_LOG(LogSplineCtrl, Warning, TEXT("ConvertToPolynomialForm = %s"), bConvertToPolynomialForm ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ASplineTestPlayerController::FlipDisplayControlPoint()
{
	bDisplayControlPoint = !bDisplayControlPoint;
	UE_LOG(LogSplineCtrl, Warning, TEXT("DisplayMiddleControlPoint = %s"), bDisplayControlPoint ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ASplineTestPlayerController::FlipDisplaySmallTangentOfInternalKnot()
{
	bDisplaySmallTangent = !bDisplaySmallTangent;
	UE_LOG(LogSplineCtrl, Warning, TEXT("DisplaySmallTangentOfInternalKnot = %s"), bDisplaySmallTangent ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ASplineTestPlayerController::FlipDisplaySmallCurvatureOfInternalKnot()
{
	bDisplaySmallCurvature = !bDisplaySmallCurvature;
	UE_LOG(LogSplineCtrl, Warning, TEXT("DisplaySmallCurvatureOfInternalKnot = %s"), bDisplaySmallCurvature ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ASplineTestPlayerController::ClearCanvas()
{
	ClearCanvasImpl();
}

void ASplineTestPlayerController::OnParamsInputChanged()
{
}

void ASplineTestPlayerController::AddControlPoint(const FVector& HitPoint)
{
	auto HitPointToControlPoint = [this](const FVector& P) -> FVector {
		static constexpr double Weight = 1.;
		return FVector(Canvas2D->FromCanvasPoint(P) * Weight, Weight);
	};
	FVector EndPoint = HitPointToControlPoint(HitPoint);
	ControlPoints.Add(EndPoint);
	Splines.Last().AddPointAtLast(EndPoint);

	ResampleCurve();
}

void ASplineTestPlayerController::ClearCanvasImpl()
{
	UE_LOG(LogTemp, Warning, TEXT("Clear Canvas"));
	for (int32 Layer = 0; Layer < Canvas2D->DisplayPoints.Num(); ++Layer) {
		Canvas2D->DisplayPoints[Layer].Array.Empty(MaxSamplePointsNum);
	}
	for (int32 Layer = 0; Layer < Canvas2D->DisplayLines.Num(); ++Layer) {
		Canvas2D->DisplayLines[Layer].Array.Empty(MaxSamplePointsNum);
	}
	for (int32 Layer = 0; Layer < Canvas2D->DisplayPolygons.Num(); ++Layer) {
		Canvas2D->DisplayPolygons[Layer].Array.Empty(MaxSamplePointsNum);
	}
	Canvas2D->ClearDrawing();
	ControlPoints.Empty(0);
	Splines.Empty(0);
	Splines.AddDefaulted();
}

void ASplineTestPlayerController::ResampleCurve()
{
	auto ControlPointToHitPoint = [this](const FVector& P) -> FVector {
		return Canvas2D->ToCanvasPoint(FVector2D(P));
	};
	UE_LOG(LogSplineCtrl, Warning, TEXT("Splines Num = %d, CtrlPoints Num = %d"),
		Splines.Num(), ControlPoints.Num());

	for (int32 Layer = 0; Layer < Canvas2D->DisplayPoints.Num(); ++Layer) {
		Canvas2D->DisplayPoints[Layer].Array.Empty(MaxSamplePointsNum);
	}
	for (int32 Layer = 0; Layer < Canvas2D->DisplayLines.Num(); ++Layer) {
		Canvas2D->DisplayLines[Layer].Array.Empty(MaxSamplePointsNum);
	}
	for (int32 Layer = 0; Layer < Canvas2D->DisplayPolygons.Num(); ++Layer) {
		Canvas2D->DisplayPolygons[Layer].Array.Empty(MaxSamplePointsNum);
	}
	Canvas2D->ClearDrawing();

	if (bDisplayControlPoint) {
		for (const FVector& P : ControlPoints) {
			Canvas2D->DisplayPoints[Splines.Num() - 1].Array.Add(ControlPointToHitPoint(P));
		}
	}
	//else {
	//	if (ControlPoints.Num() > 0) {
	//		Canvas2D->DisplayPoints[Splines.Num() - 1].Array.Add(ControlPointToHitPoint(ControlPoints[0]));
	//	}
	//	if (ControlPoints.Num() > 1) {
	//		Canvas2D->DisplayPoints[Splines.Num() - 1].Array.Add(ControlPointToHitPoint(ControlPoints.Last()));
	//	}
	//}

	int32 SplineLayer = 0;

	for (int32 i = 0; i < Splines.Num(); ++i) {
		UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d] Num = %d"),
			i, Splines[i].GetCtrlPointNum());

		Canvas2D->DrawPoints(i);

		TArray<FVector4> SpCtrlPoints; TArray<double> SpParams;
		Splines[i].GetOpenFormPointsAndParams(SpCtrlPoints, SpParams);

		if (Splines[i].GetCtrlPointNum() < 2) {
			continue;
		}
		const auto& ParamRange = Splines[i].GetParamRange();

		for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
			UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].Points[%d] = <%s, %.6lf>"),
				i, j, *SpCtrlPoints[j].ToString(), SpParams[j]);
			UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].Tangents[%d] = <%s, %.6lf>"),
				i, j, *Splines[i].GetTangent(ParamRange.Get<1>()).ToString(), Splines[i].GetTangent(ParamRange.Get<1>()).Size());
			UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].PrincipalCurvatures[%d] = <%.6lf>"),
				i, j, Splines[i].GetPrincipalCurvature(ParamRange.Get<1>(), 0));
		}

		for (double T = ParamRange.Get<0>(); T <= ParamRange.Get<1>(); T += SamplePointDT) {
			FVector LinePoint = ControlPointToHitPoint(Splines[i].GetPosition(T));
			Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);
			int32 AdditionalLayer = 0;
			if (bDisplaySmallTangent) {
				++AdditionalLayer;
				FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i].GetTangent(T).GetSafeNormal() * 100.);
				Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(TangentPoint);
			}
			if (bDisplaySmallCurvature) {
				++AdditionalLayer;
				Canvas2D->ToCanvasPoint(FVector2D(Splines[i].GetTangent(T)).GetRotated(90.));
				FVector CurvaturePoint = LinePoint + ControlPointToHitPoint(Splines[i].GetTangent(T).GetSafeNormal() * 1000.).RotateAngleAxis(90., FVector::BackwardVector) * Splines[i].GetPrincipalCurvature(T, 0);
				Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(CurvaturePoint);
			}
		}
		Canvas2D->DrawLines(SplineLayer);
		++SplineLayer;
		if (bDisplaySmallTangent) {
			Canvas2D->DrawLines(SplineLayer);
			++SplineLayer;
		}
		if (bDisplaySmallCurvature) {
			Canvas2D->DrawLines(SplineLayer);
			++SplineLayer;
		}
	}
}

void ASplineTestPlayerController::AddControlPointEvent(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
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
	AddControlPoint(HitPoint);
}

void ASplineTestPlayerController::AddNewSplineEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	Splines.AddDefaulted();
}

void ASplineTestPlayerController::ClearCanvasEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	ClearCanvas();
}

void ASplineTestPlayerController::ChangeConcatTypeToPoint(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Curve To Point"));
	ChangeConcatType(ESplineConcatType::ToPoint);
}

void ASplineTestPlayerController::ChangeConcatTypeToCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Curve To Curve"));
	ChangeConcatType(ESplineConcatType::ToCurve);
}

void ASplineTestPlayerController::FlipConvertToPolynomialFormEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipConvertToPolynomialForm();
}

void ASplineTestPlayerController::FlipDisplayControlPointEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplayControlPoint();
}

void ASplineTestPlayerController::FlipDisplaySmallTangentOfInternalKnotEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallTangentOfInternalKnot();
}

void ASplineTestPlayerController::FlipDisplaySmallCurvatureOfInternalKnotEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallCurvatureOfInternalKnot();
}