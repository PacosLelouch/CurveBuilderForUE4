// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/


#include "SplineTestPlayerController.h"
#include "CGDemoCanvas2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Logging/LogMacros.h"
#include "Engine.h"

#define GetValueRef GetValue().Get

DEFINE_LOG_CATEGORY_STATIC(LogSplineCtrl, Warning, All)

static const double PointDistSqr = 16.0;
static const double NodeDistSqr = 100.0;

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

	InputYawScale = 0.;
	InputPitchScale = 0.;
	InputRollScale = 0.;
}

void ASplineTestPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);
	NearestPoint.Reset();

	Canvas2D->DisplayPoints[1].Array.Empty(Canvas2D->DisplayPoints[1].Array.Num());
	Canvas2D->DisplayPoints[2].Array.Empty(Canvas2D->DisplayPoints[2].Array.Num());

	if (!bPressedLeftMouseButton) {
		bHoldingPoint = false;
	}

	float MouseX, MouseY;
	FVector WorldPos, WorldDir;

	if (GetMousePosition(MouseX, MouseY) && DeprojectScreenPositionToWorld(MouseX, MouseY, WorldPos, WorldDir)) {
		float Distance = 0;
		FVector HitPoint(0, 0, 0);
		bool bHit = TraceToCanvas(Distance, HitPoint, WorldPos, WorldDir);
		if (bHit) {
			FVector CtrlPoint = HitPointToControlPoint(HitPoint);
			if (Splines.Num()) {
				auto& Spline = Splines.Last();

				double Param = -1;
				if (Spline.FindParamByPosition(Param, CtrlPoint, PointDistSqr)) {
					FVector NearestPos = Spline.GetPosition(Param);
					if (FVector::DistSquared(NearestPos, CtrlPoint) < PointDistSqr) {
						UE_LOG(LogSplineCtrl, Warning, TEXT("Param = %.6lf"), Param);
						NearestPoint.Emplace(NearestPos);
					}
				}

				NearestNode = Spline.FindNodeByPosition(CtrlPoint, 0, NodeDistSqr);

				if (SelectedNode) {
					if (bPressedLeftMouseButton) {
						FVector SelectedPos = TVecLib<4>::Projection(SelectedNode->GetValueRef().Pos);
						if (bHoldingPoint || (!bHoldingPoint && FVector::DistSquared(SelectedPos, CtrlPoint) < NodeDistSqr)) {
							bHoldingPoint = true;
							Spline.AdjustCtrlPointPos(SelectedNode, CtrlPoint, 0);
							ResampleCurve();
						}
					}
				}
			}


			if (NearestPoint) {
				Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(NearestPoint.GetValue()));
			}
			if (NearestNode) {
				Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(NearestNode->GetValueRef().Pos)));
			}
		}
	}

	if (SelectedNode) {
		Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(SelectedNode->GetValueRef().Pos)));
	}
	Canvas2D->DrawPoints(1);
	Canvas2D->DrawPoints(2);
}

void ASplineTestPlayerController::BindOnLeftMouseButtonPressed()
{
	OnLeftMouseButtonPressed.AddDynamic(this, &ASplineTestPlayerController::PressLeftMouseButton);
}

void ASplineTestPlayerController::BindOnLeftMouseButtonReleased()
{
	OnLeftMouseButtonReleased.AddDynamic(this, &ASplineTestPlayerController::ReleaseLeftMouseButton);
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
	OnCtrlAndKey3Released.AddDynamic(this, &ASplineTestPlayerController::FlipDisplaySmallCurvatureOfInternalKnotEvent);
}

void ASplineTestPlayerController::BindOnCtrlAndKey4Released()
{
	OnCtrlAndKey4Released.AddDynamic(this, &ASplineTestPlayerController::SplitSplineAtCenterEvent);
}

void ASplineTestPlayerController::BindOnCtrlAndKey5Released()
{
	// Do nothing.
	OnCtrlAndKey5Released.AddDynamic(this, &ASplineTestPlayerController::FlipConvertToBezierEvent);
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

void ASplineTestPlayerController::FlipConvertToBezier()
{
	bConvertToBezier = !bConvertToBezier;
	UE_LOG(LogSplineCtrl, Warning, TEXT("ConvertToBezier = %s"), bConvertToBezier ? TEXT("true") : TEXT("false"));
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

void ASplineTestPlayerController::SplitSplineAtCenter()
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
	Last.GetKnotIntervals(Params);

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

void ASplineTestPlayerController::AddControlPoint(const FVector& HitPoint)
{
	FVector EndPoint = HitPointToControlPoint(HitPoint);
	ControlPoints.Add(EndPoint);

	double Param = -1;
	if (Splines.Last().FindParamByPosition(Param, EndPoint, PointDistSqr)) {
		//UE_LOG(LogBezierStringTest, Warning, TEXT("Param = %.6lf"), Param);
		Splines.Last().AddPointWithParamWithoutChangingShape(Param);
	}
	else {
		Splines.Last().AddPointAtLast(EndPoint);
	}

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
	if (!bConvertToBezier) {
		CurLayer = ResampleBSpline(CurLayer);
	}
	else if (bConvertToBezier) {
		CurLayer = ResampleBezier(CurLayer);
	}
}

int32 ASplineTestPlayerController::ResampleBezier(int32 FirstLineLayer)
{
	int32 BezierLayer = FirstLineLayer;
	BezierCurves.Empty(Splines.Num());
	for (int32 i = 0; i < Splines.Num(); ++i) {
		const auto& Spline = Splines[i];
		auto& Beziers = BezierCurves.AddDefaulted_GetRef();
		Spline.ToBezierCurves(Beziers);
		UE_LOG(LogSplineCtrl, Warning, TEXT("Beziers[%d] Num = %d"),
			i, Beziers.Num());
		
		for (int32 j = 0; j < Beziers.Num(); ++j) {
			for (int32 k = 0; k < Beziers[j].CurveOrder(); ++k) {
				Canvas2D->DisplayPoints[BezierLayer % Canvas2D->PointLayerConfig.MaxLayerCount].Array.Add(ControlPointToHitPoint(Beziers[j].GetPoint(k)));
			}
			for (double T = 0.; T <= 1.; T += SamplePointDT) {
				FVector LinePoint = ControlPointToHitPoint(Beziers[j].GetPosition(T));
				Canvas2D->DisplayLines[BezierLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);
			}
			Canvas2D->DrawPoints(BezierLayer);
			Canvas2D->DrawLines(BezierLayer);
			++BezierLayer;
		}
	}
	return BezierLayer;
}

int32 ASplineTestPlayerController::ResampleBSpline(int32 FirstLineLayer)
{
	UE_LOG(LogSplineCtrl, Warning, TEXT("Splines Num = %d, CtrlPoints Num = %d"),
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
		UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d] CtrlPointNum = %d, KnotNum = %d"),
			i, Splines[i].GetCtrlPointNum(), Splines[i].GetKnotNum());

		TArray<FVector4> SpCtrlPoints; TArray<double> SpParams;
		Splines[i].GetCtrlPoints(SpCtrlPoints);
		Splines[i].GetKnotIntervals(SpParams);

		if (Splines[i].GetCtrlPointNum() < 2) {
			if (Splines[i].GetCtrlPointNum() > 0) {
				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(Splines[i].FirstNode()->GetValueRef().Pos)));
			}
			continue;
		}
		const auto& ParamRange = Splines[i].GetParamRange();


		if (bDisplayControlPoint) {
			for (const auto& PH : SpCtrlPoints) {
				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
				Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
			}
			Canvas2D->DrawLines(SplineLayer);
			++SplineLayer;
			for (const auto& T : SpParams) {
				const auto& P = Splines[i].GetPosition(T);
				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(P));
			}
		}

		for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
			UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].Points[%d] = <%s>"),
				i, j, *SpCtrlPoints[j].ToString());
		}
		for (int32 j = 0; j < SpParams.Num(); ++j) {
			UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].Knots[%d] = <%s, t = %.6lf>"),
				i, j, *Splines[i].GetPosition(SpParams[j]).ToString(), SpParams[j]);
			if (bDisplaySmallTangent) {
				UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].Tangents[%d] = <%s, size = %.6lf>"),
					i, j, *Splines[i].GetTangent(SpParams[j]).ToString(), Splines[i].GetTangent(SpParams[j]).Size());
			}
			if (bDisplaySmallCurvature) {
				UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].PlanCurvatures[%d] = <%.6lf>"),
					i, j, Splines[i].GetPlanCurvature(SpParams[j], 0));
			}
		}

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
				FVector CurvaturePoint = LinePoint + ControlPointToHitPoint(Splines[i].GetTangent(T).GetSafeNormal() * 1000.).RotateAngleAxis(90., FVector::BackwardVector) * Splines[i].GetPlanCurvature(T, 0);
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
	Canvas2D->DrawPoints(0);
	return SplineLayer;
}

void ASplineTestPlayerController::PressLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	bPressedLeftMouseButton = true;
}

void ASplineTestPlayerController::ReleaseLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	bPressedLeftMouseButton = false;
	if (SelectedNode) {
		SelectedNode = nullptr;
	}
	if (NearestNode) {
		SelectedNode = NearestNode;
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

void ASplineTestPlayerController::FlipConvertToBezierEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipConvertToBezier();
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

void ASplineTestPlayerController::SplitSplineAtCenterEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	SplitSplineAtCenter();
}

FVector ASplineTestPlayerController::ControlPointToHitPoint(const FVector& P)
{
	return Canvas2D->ToCanvasPoint(FVector2D(P));
}

FVector ASplineTestPlayerController::HitPointToControlPoint(const FVector& P)
{
	static constexpr double Weight = 1.;
	return FVector(Canvas2D->FromCanvasPoint(P) * Weight, Weight);
}

#undef GetValueRef
