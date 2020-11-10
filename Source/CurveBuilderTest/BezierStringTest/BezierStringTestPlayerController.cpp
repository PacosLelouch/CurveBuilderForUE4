// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/


#include "BezierStringTestPlayerController.h"
#include "CGDemoCanvas2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Logging/LogMacros.h"
#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBezierStringTest, Warning, All);

DEFINE_LOG_CATEGORY(LogBezierStringTest);

ABezierStringTestPlayerController::ABezierStringTestPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABezierStringTestPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MaxSamplePointsNum = FMath::CeilToInt((Canvas2D->CanvasBoxYZ.Max.Y - Canvas2D->CanvasBoxYZ.Min.Y) / SamplePointDT) + 1;
	Splines.Empty();
	Splines.AddDefaulted();
}

void ABezierStringTestPlayerController::BindOnRightMouseButtonReleased()
{
	OnRightMouseButtonReleased.AddDynamic(this, &ABezierStringTestPlayerController::AddControlPointEvent);
}

void ABezierStringTestPlayerController::BindOnCtrlAndKey1Released()
{
	//OnCtrlAndKey1Released.AddDynamic(this, &ABezierStringTestPlayerController::ChangeConcatTypeToPoint);
	OnCtrlAndKey1Released.AddDynamic(this, &ABezierStringTestPlayerController::FlipDisplayControlPointEvent);
}

void ABezierStringTestPlayerController::BindOnCtrlAndKey2Released()
{
	//OnCtrlAndKey2Released.AddDynamic(this, &ABezierStringTestPlayerController::ChangeConcatTypeToCurve);
	OnCtrlAndKey2Released.AddDynamic(this, &ABezierStringTestPlayerController::FlipDisplaySmallTangentOfInternalKnotEvent);
}

void ABezierStringTestPlayerController::BindOnCtrlAndKey3Released()
{
	OnCtrlAndKey3Released.AddDynamic(this, &ABezierStringTestPlayerController::FlipDisplaySmallCurvatureOfInternalKnotEvent);
}

void ABezierStringTestPlayerController::BindOnCtrlAndKey4Released()
{
	OnCtrlAndKey4Released.AddDynamic(this, &ABezierStringTestPlayerController::SplitSplineAtCenterEvent);
}

void ABezierStringTestPlayerController::BindOnCtrlAndKey5Released()
{
	// Do nothing.
	OnCtrlAndKey5Released.AddDynamic(this, &ABezierStringTestPlayerController::FlipConvertToBezierEvent);
}

void ABezierStringTestPlayerController::BindOnCtrlAndKey0Released()
{
	//Super::BindOnCtrlAndKey0Released();
	OnCtrlAndKey0Released.AddDynamic(this, &ABezierStringTestPlayerController::ClearCanvasEvent);
}

void ABezierStringTestPlayerController::BindOnEnterReleased()
{
	Super::BindOnEnterReleased();
	OnEnterReleased.AddDynamic(this, &ABezierStringTestPlayerController::AddNewSplineEvent);
}

//void ABezierStringTestPlayerController::ChangeConcatType(ESplineConcatType Type)
//{
//	ConcatType = Type;
//	ClearCanvas();
//}

void ABezierStringTestPlayerController::FlipConvertToBezier()
{
	bConvertToBezier = !bConvertToBezier;
	UE_LOG(LogBezierStringTest, Warning, TEXT("ConvertToBezier = %s"), bConvertToBezier ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ABezierStringTestPlayerController::FlipDisplayControlPoint()
{
	bDisplayControlPoint = !bDisplayControlPoint;
	UE_LOG(LogBezierStringTest, Warning, TEXT("DisplayMiddleControlPoint = %s"), bDisplayControlPoint ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ABezierStringTestPlayerController::FlipDisplaySmallTangentOfInternalKnot()
{
	bDisplaySmallTangent = !bDisplaySmallTangent;
	UE_LOG(LogBezierStringTest, Warning, TEXT("DisplaySmallTangentOfInternalKnot = %s"), bDisplaySmallTangent ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ABezierStringTestPlayerController::FlipDisplaySmallCurvatureOfInternalKnot()
{
	bDisplaySmallCurvature = !bDisplaySmallCurvature;
	UE_LOG(LogBezierStringTest, Warning, TEXT("DisplaySmallCurvatureOfInternalKnot = %s"), bDisplaySmallCurvature ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void ABezierStringTestPlayerController::ClearCanvas()
{
	ClearCanvasImpl();
}

void ABezierStringTestPlayerController::OnParamsInputChanged()
{
}

void ABezierStringTestPlayerController::SplitSplineAtCenter()
{
	if (Splines.Num() == 0) {
		return;
	}
	const auto& ParamRange = Splines.Last().GetParamRange();
	const auto& Last = Splines.Pop();
	int32 FirstIdx = Splines.AddDefaulted();
	int32 SecondIdx = Splines.AddDefaulted();

	auto& First = Splines[FirstIdx];
	auto& Second = Splines[SecondIdx];

	TArray<FVector4> Poss;
	TArray<double> Params;
	Last.GetCtrlPoints(Poss);
	//Last.GetKnotIntervals(Params);

	//Last.Split(First, Second, Params.Num() > 0 ? Params[Params.Num() > 2 ? 2 : Params.Num() - 1] : 0.);
	Last.Split(First, Second, 0.5 * (ParamRange.Get<0>() + ParamRange.Get<1>()));
	if (Second.GetCtrlPointNum() == 0) {
		Splines.Pop(false);
	}
	if (First.GetCtrlPointNum() == 0) {
		Splines.Pop(false);
	}

	ResampleCurve();
}

void ABezierStringTestPlayerController::AddControlPoint(const FVector& HitPoint)
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

void ABezierStringTestPlayerController::ClearCanvasImpl()
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

void ABezierStringTestPlayerController::ResampleCurve()
{
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

	int32 CurLayer = 0;
	//if (!bConvertToBezier) {
		CurLayer = ResampleBSpline(CurLayer);
	//}
	//else if (bConvertToBezier) {
	//	CurLayer = ResampleBezier(CurLayer);
	//}
}

//int32 ABezierStringTestPlayerController::ResampleBezier(int32 FirstLineLayer)
//{
//	auto ControlPointToHitPoint = [this](const FVector& P) -> FVector {
//		return Canvas2D->ToCanvasPoint(FVector2D(P));
//	};
//
//	int32 BezierLayer = FirstLineLayer;
//	BezierCurves.Empty(Splines.Num());
//	for (int32 i = 0; i < Splines.Num(); ++i) {
//		const auto& Spline = Splines[i];
//		auto& Beziers = BezierCurves.AddDefaulted_GetRef();
//		Spline.ToBezierString(Beziers);
//		UE_LOG(LogBezierStringTest, Warning, TEXT("Beziers[%d] Num = %d"),
//			i, Beziers.Num());
//		
//		for (int32 j = 0; j < Beziers.Num(); ++j) {
//			for (int32 k = 0; k < Beziers[j].CurveOrder(); ++k) {
//				Canvas2D->DisplayPoints[BezierLayer % Canvas2D->PointLayerConfig.MaxLayerCount].Array.Add(ControlPointToHitPoint(Beziers[j].GetPoint(k)));
//			}
//			for (double T = 0.; T <= 1.; T += SamplePointDT) {
//				FVector LinePoint = ControlPointToHitPoint(Beziers[j].GetPosition(T));
//				Canvas2D->DisplayLines[BezierLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);
//			}
//			Canvas2D->DrawPoints(BezierLayer);
//			Canvas2D->DrawLines(BezierLayer);
//			++BezierLayer;
//		}
//	}
//	return BezierLayer;
//}

int32 ABezierStringTestPlayerController::ResampleBSpline(int32 FirstLineLayer)
{
	auto ControlPointToHitPoint = [this](const FVector& P) -> FVector {
		return Canvas2D->ToCanvasPoint(FVector2D(P));
	};
	UE_LOG(LogBezierStringTest, Warning, TEXT("Splines Num = %d, CtrlPoints Num = %d"),
		Splines.Num(), ControlPoints.Num());

	//if(bDisplayControlPoint) {
	//	for (const FVector& P : ControlPoints) {
	//		Canvas2D->DisplayPoints[Splines.Num() - 1].Array.Add(ControlPointToHitPoint(P));
	//	}
	//}
	//else {
	//	if (ControlPoints.Num() > 0) {
	//		Canvas2D->DisplayPoints[Splines.Num() - 1].Array.Add(ControlPointToHitPoint(ControlPoints[0]));
	//	}
	//	if (ControlPoints.Num() > 1) {
	//		Canvas2D->DisplayPoints[Splines.Num() - 1].Array.Add(ControlPointToHitPoint(ControlPoints.Last()));
	//	}
	//}

	int32 SplineLayer = FirstLineLayer;

	for (int32 i = 0; i < Splines.Num(); ++i) {
		UE_LOG(LogBezierStringTest, Warning, TEXT("Splines[%d] CtrlPointNum = %d"),
			i, Splines[i].GetCtrlPointNum());

		TArray<FVector4> SpCtrlPoints; //TArray<double> SpParams;
		Splines[i].GetCtrlPoints(SpCtrlPoints);
		//Splines[i].GetKnotIntervals(SpParams);

		if (Splines[i].GetCtrlPointNum() < 2) {
			Canvas2D->DrawPoints(i);
			continue;
		}
		const auto& ParamRange = Splines[i].GetParamRange();


		if (bDisplayControlPoint) {
			for (const auto& PH : SpCtrlPoints) {
				Canvas2D->DisplayPoints[i].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
			}
			//for (const auto& T : SpParams) {
			//	const auto& P = Splines[i].GetPosition(T);
			//	Canvas2D->DisplayPoints[i].Array.Add(ControlPointToHitPoint(P));
			//}
		}

		for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
			UE_LOG(LogBezierStringTest, Warning, TEXT("Splines[%d].Points[%d] = <%s>"),
				i, j, *SpCtrlPoints[j].ToString());
		}
		//for (int32 j = 0; j < SpParams.Num(); ++j) {
		//	UE_LOG(LogBezierStringTest, Warning, TEXT("Splines[%d].Knots[%d] = <%s, t = %.6lf>"),
		//		i, j, *Splines[i].GetPosition(SpParams[j]).ToString(), SpParams[j]);
		//	if (bDisplaySmallTangent) {
		//		UE_LOG(LogBezierStringTest, Warning, TEXT("Splines[%d].Tangents[%d] = <%s, size = %.6lf>"),
		//			i, j, *Splines[i].GetTangent(SpParams[j]).ToString(), Splines[i].GetTangent(SpParams[j]).Size());
		//	}
		//	if (bDisplaySmallCurvature) {
		//		UE_LOG(LogBezierStringTest, Warning, TEXT("Splines[%d].PrincipalCurvatures[%d] = <%.6lf>"),
		//			i, j, Splines[i].GetPrincipalCurvature(SpParams[j], 0));
		//	}
		//}

		int32 SegNumDbl = FMath::CeilToDouble((ParamRange.Get<1>() - ParamRange.Get<0>()) / SamplePointDT);
		//for (double T = ParamRange.Get<0>(); T <= ParamRange.Get<1>(); T += SamplePointDT) {
		for(int32 Cnt = 0; Cnt <= SegNumDbl; ++Cnt) {
			double T = ParamRange.Get<0>() + (ParamRange.Get<1>() - ParamRange.Get<0>()) * static_cast<double>(Cnt) / static_cast<double>(SegNumDbl);
			FVector LinePoint = ControlPointToHitPoint(Splines[i].GetPosition(T));
			Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);

			int32 AdditionalLayer = 0;
			if (bDisplaySmallTangent) {
				++AdditionalLayer;
				//FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i].GetTangent(T).GetSafeNormal() * 100.);
				FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i].GetTangent(T) * 1. / 3.);
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

		Canvas2D->DrawPoints(i);
	}
	return SplineLayer;
}

void ABezierStringTestPlayerController::AddControlPointEvent(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
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

void ABezierStringTestPlayerController::AddNewSplineEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	Splines.AddDefaulted();
}

void ABezierStringTestPlayerController::ClearCanvasEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	ClearCanvas();
}

void ABezierStringTestPlayerController::ChangeConcatTypeToPoint(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Curve To Point"));
	//ChangeConcatType(ESplineConcatType::ToPoint);
}

void ABezierStringTestPlayerController::ChangeConcatTypeToCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Curve To Curve"));
	//ChangeConcatType(ESplineConcatType::ToCurve);
}

void ABezierStringTestPlayerController::FlipConvertToBezierEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipConvertToBezier();
}

void ABezierStringTestPlayerController::FlipDisplayControlPointEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplayControlPoint();
}

void ABezierStringTestPlayerController::FlipDisplaySmallTangentOfInternalKnotEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallTangentOfInternalKnot();
}

void ABezierStringTestPlayerController::FlipDisplaySmallCurvatureOfInternalKnotEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallCurvatureOfInternalKnot();
}

void ABezierStringTestPlayerController::SplitSplineAtCenterEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	SplitSplineAtCenter();
}
