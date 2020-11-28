// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "GraphTestPlayerController.h"
#include "CGDemoCanvas2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Logging/LogMacros.h"
#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGraphTest, Warning, All);

DEFINE_LOG_CATEGORY(LogGraphTest);

static const double PointDistSqr = 16.0;
static const double NodeDistSqr = 100.0;

AGraphTestPlayerController::AGraphTestPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = APlayerCameraManager::StaticClass();
}

void AGraphTestPlayerController::BeginPlay()
{
	Super::BeginPlay();
	MaxSamplePointsNum = FMath::CeilToInt((Canvas2D->CanvasBoxYZ.Max.Y - Canvas2D->CanvasBoxYZ.Min.Y) / SamplePointDT) + 1;
	Graph.Empty();
	Graph.AddDefaulted(NewSplineType);

	FixedTransform = FTransform(AController::GetControlRotation(), GetPawn()->GetActorLocation());
	InputYawScale = 0.;
	InputPitchScale = 0.;
	InputRollScale = 0.;
}

void AGraphTestPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);
	NearestPoint.Reset();
	NearestSpline = nullptr;

	if (FixedTransform) {
	}

	if (!bPressedLeftMouseButton) {
		HoldingPointType.Reset();
	}

	Canvas2D->DisplayLines[16].Array.Empty(Canvas2D->DisplayLines[16].Array.Num());
	Canvas2D->DisplayPoints[1].Array.Empty(Canvas2D->DisplayPoints[1].Array.Num());
	Canvas2D->DisplayPoints[2].Array.Empty(Canvas2D->DisplayPoints[2].Array.Num());

	float MouseX, MouseY;
	FVector WorldPos, WorldDir;

	if (GetMousePosition(MouseX, MouseY) && DeprojectScreenPositionToWorld(MouseX, MouseY, WorldPos, WorldDir)) {
		float Distance = 0;
		FVector HitPoint(0, 0, 0);
		bool bHit = TraceToCanvas(Distance, HitPoint, WorldPos, WorldDir);
		if (bHit) {
			FVector CtrlPoint = HitPointToControlPoint(HitPoint);
			if (Graph.Num()) {
				TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
				Graph.GetSplines(Splines);
				for (int32 i = Splines.Num() - 1; i >= 0; --i) {
					auto& Spline = *Splines[i].Pin().Get();

					double Param = -1;
					if (Spline.FindParamByPosition(Param, CtrlPoint, PointDistSqr)) {
						FVector NearestPos = Spline.GetPosition(Param);
						if (FVector::DistSquared(NearestPos, CtrlPoint) < PointDistSqr) {
							//UE_LOG(LogGraphTest, Warning, TEXT("Param = %.6lf"), Param);
							NearestPoint.Emplace(NearestPos);
							NearestSpline = Splines[i];
							break;
						}
					}
				}
				if (SelectedSpline.IsValid()) {
					auto& Spline = *SelectedSpline.Pin().Get();
					CurSplineType = Spline.GetType();

					switch (CurSplineType) {
					case ESplineType::BezierString:
					{
						FSpatialBezierString3& SplineBezierString = static_cast<FSpatialBezierString3&>(Spline);
						NearestNodeRaw = SplineBezierString.FindNodeByPosition(CtrlPoint, 0, NodeDistSqr);
						FSpatialBezierString3::FPointNode* NearestNode = static_cast<FSpatialBezierString3::FPointNode*>(NearestNodeRaw);
						FSpatialBezierString3::FPointNode* SelectedNode = static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw);

						if (SelectedNode) {
							if (bPressedLeftMouseButton) {
								FVector SelectedPos = TVecLib<4>::Projection(SelectedNode->GetValue().Pos);
								FVector SelectedPrevPos = TVecLib<4>::Projection(SelectedNode->GetValue().PrevCtrlPointPos);
								FVector SelectedNextPos = TVecLib<4>::Projection(SelectedNode->GetValue().NextCtrlPointPos);
								if ((HoldingPointType && HoldingPointType.GetValue() == ESelectedNodeCtrlPointType::Current) ||
									(!HoldingPointType && FVector::DistSquared(SelectedPos, CtrlPoint) < NodeDistSqr)) {
									HoldingPointType = ESelectedNodeCtrlPointType::Current;
									Graph.AdjustCtrlPointPos(SelectedPos, CtrlPoint, SelectedSpline, 1, 0, 0, NodeDistSqr);
									//SplineBezierString.AdjustCtrlPointPos(SelectedNode, CtrlPoint, 0);
									ResampleCurve();
								}
								else if ((HoldingPointType && HoldingPointType.GetValue() == ESelectedNodeCtrlPointType::Next) ||
									(!HoldingPointType && FVector::DistSquared(SelectedNextPos, CtrlPoint) < NodeDistSqr)) {
									HoldingPointType = ESelectedNodeCtrlPointType::Next;
									Graph.AdjustCtrlPointPos(SelectedNextPos, CtrlPoint, SelectedSpline, 1, 1, 0, NodeDistSqr);
									//SplineBezierString.AdjustCtrlPointTangent(SelectedNode, CtrlPoint, true, 0);
									Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(SelectedNode->GetValue().PrevCtrlPointPos)));
									ResampleCurve();
								}
								else if ((HoldingPointType && HoldingPointType.GetValue() == ESelectedNodeCtrlPointType::Previous) ||
									(!HoldingPointType && FVector::DistSquared(SelectedPrevPos, CtrlPoint) < NodeDistSqr)) {
									HoldingPointType = ESelectedNodeCtrlPointType::Previous;
									Graph.AdjustCtrlPointPos(SelectedPrevPos, CtrlPoint, SelectedSpline, 1, -1, 0, NodeDistSqr);
									//SplineBezierString.AdjustCtrlPointTangent(SelectedNode, CtrlPoint, false, 0);
									Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(SelectedNode->GetValue().NextCtrlPointPos)));
									ResampleCurve();
								}
							}
						}
					}
					break;
					case ESplineType::ClampedBSpline:
					{
						FSpatialBSpline3& SplineBSpline = static_cast<FSpatialBSpline3&>(Spline);
						NearestNodeRaw = SplineBSpline.FindNodeByPosition(CtrlPoint, 0, NodeDistSqr);
						FSpatialBSpline3::FPointNode* NearestNode = static_cast<FSpatialBSpline3::FPointNode*>(NearestNodeRaw);
						FSpatialBSpline3::FPointNode* SelectedNode = static_cast<FSpatialBSpline3::FPointNode*>(SelectedNodeRaw);
						NearestNode = SplineBSpline.FindNodeByPosition(CtrlPoint, 0, NodeDistSqr);

						if (SelectedNode) {
							if (bPressedLeftMouseButton) {
								FVector SelectedPos = TVecLib<4>::Projection(SelectedNode->GetValue().Pos);
								if (HoldingPointType || (!HoldingPointType && FVector::DistSquared(SelectedPos, CtrlPoint) < NodeDistSqr)) {
									HoldingPointType = ESelectedNodeCtrlPointType::Current;
									Graph.AdjustCtrlPointPos(SelectedPos, CtrlPoint, SelectedSpline, 1, 0, 0, NodeDistSqr);
									//SplineBSpline.AdjustCtrlPointPos(SelectedNode, CtrlPoint, 0);
									ResampleCurve();
								}
							}
						}
					}
					break;
					}
				}
			}


			if (NearestPoint) {
				Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(NearestPoint.GetValue()));
			}
			if (NearestNodeRaw) {
				switch (CurSplineType) {
				case ESplineType::BezierString:
					Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(NearestNodeRaw)->GetValue().Pos)));
					break;
				}
			}
		}
	}

	if (SelectedNodeRaw) {
		switch (CurSplineType) {
		case ESplineType::BezierString:
			Canvas2D->DisplayLines[16].Array.Add(ControlPointToHitPoint(
				TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValue().PrevCtrlPointPos)));
			Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(
				TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValue().PrevCtrlPointPos)));
			Canvas2D->DisplayLines[16].Array.Add(ControlPointToHitPoint(
				TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValue().Pos)));
			Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(
				TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValue().Pos)));
			Canvas2D->DisplayLines[16].Array.Add(ControlPointToHitPoint(
				TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValue().NextCtrlPointPos)));
			Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(
				TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValue().NextCtrlPointPos)));
			break;
		case ESplineType::ClampedBSpline:
			Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(
				TVecLib<4>::Projection(static_cast<FSpatialBSpline3::FPointNode*>(SelectedNodeRaw)->GetValue().Pos)));
			break;
		}
	}
	if (CurSplineType == ESplineType::BezierString) {
		Canvas2D->DrawLines(16);
	}
	Canvas2D->DrawPoints(1);
	Canvas2D->DrawPoints(2);
}

void AGraphTestPlayerController::BindOnLeftMouseButtonPressed()
{
	OnLeftMouseButtonPressed.AddDynamic(this, &AGraphTestPlayerController::PressLeftMouseButton);
}

void AGraphTestPlayerController::BindOnLeftMouseButtonReleased()
{
	OnLeftMouseButtonReleased.AddDynamic(this, &AGraphTestPlayerController::ReleaseLeftMouseButton);
}

void AGraphTestPlayerController::BindOnRightMouseButtonReleased()
{
	OnRightMouseButtonReleased.AddDynamic(this, &AGraphTestPlayerController::AddControlPointEvent);
}

void AGraphTestPlayerController::BindOnCtrlAndKey1Released()
{
	OnCtrlAndKey1Released.AddDynamic(this, &AGraphTestPlayerController::FlipDisplayControlPointEvent);
}

void AGraphTestPlayerController::BindOnCtrlAndKey2Released()
{
	OnCtrlAndKey2Released.AddDynamic(this, &AGraphTestPlayerController::FlipDisplaySmallTangentEvent);
}

void AGraphTestPlayerController::BindOnCtrlAndKey3Released()
{
	OnCtrlAndKey3Released.AddDynamic(this, &AGraphTestPlayerController::FlipDisplaySmallCurvatureEvent);
}

void AGraphTestPlayerController::BindOnCtrlAndKey4Released()
{
	//OnCtrlAndKey4Released.AddDynamic(this, &AGraphTestPlayerController::SplitSplineAtCenterEvent);
	OnCtrlAndKey4Released.AddDynamic(this, &AGraphTestPlayerController::ReverseSelectedSplineTypeEvent);
}

void AGraphTestPlayerController::BindOnCtrlAndKey5Released()
{
	//OnCtrlAndKey5Released.AddDynamic(this, &AGraphTestPlayerController::RemakeBezierC2Event);
	OnCtrlAndKey5Released.AddDynamic(this, &AGraphTestPlayerController::FlipSelectedSplineTypeEvent);
}

void AGraphTestPlayerController::BindOnCtrlAndKey0Released()
{
	//Super::BindOnCtrlAndKey0Released();
	OnCtrlAndKey0Released.AddDynamic(this, &AGraphTestPlayerController::ClearCanvasEvent);
}

void AGraphTestPlayerController::BindOnEnterReleased()
{
	Super::BindOnEnterReleased();
	OnEnterReleased.AddDynamic(this, &AGraphTestPlayerController::AddNewSplineAfterSelectedSplineEvent);
}

void AGraphTestPlayerController::RemakeBezierC2()
{
	if (Graph.Num()) {
		TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
		Graph.GetSplines(Splines);
		for (auto& SplinePtr : Splines) {
			if (!SplinePtr.IsValid()) {
				continue;
			}
			auto& Spline = *SplinePtr.Pin().Get();
			switch (Spline.GetType()) {
			case ESplineType::BezierString:
			{
				auto& SplineBezierString = static_cast<FSpatialBezierString3&>(Spline);
				SplineBezierString.RemakeC2();
				for (FSpatialBezierString3::FPointNode* Node = SplineBezierString.FirstNode(); Node; Node = Node->GetNextNode()) {
					Node->GetValue().Continuity = NewPointContinuityInit;
				}
			}
			break;
			}
		}
		ResampleCurve();
	}
}

void AGraphTestPlayerController::FlipDisplayControlPoint()
{
	bDisplayControlPoint = !bDisplayControlPoint;
	UE_LOG(LogGraphTest, Warning, TEXT("DisplayMiddleControlPoint = %s"), bDisplayControlPoint ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void AGraphTestPlayerController::FlipDisplaySmallTangent()
{
	bDisplaySmallTangent = !bDisplaySmallTangent;
	UE_LOG(LogGraphTest, Warning, TEXT("DisplaySmallTangent = %s"), bDisplaySmallTangent ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void AGraphTestPlayerController::FlipDisplaySmallCurvature()
{
	bDisplaySmallCurvature = !bDisplaySmallCurvature;
	UE_LOG(LogGraphTest, Warning, TEXT("DisplaySmallCurvature = %s"), bDisplaySmallCurvature ? TEXT("true") : TEXT("false"));
	ResampleCurve();
}

void AGraphTestPlayerController::ClearCanvas()
{
	ClearCanvasImpl();
}

void AGraphTestPlayerController::OnParamsInputChanged()
{
}

void AGraphTestPlayerController::SplitSplineAtCenter()
{
	//if (Graph.Num() == 0) {
	//	return;
	//}
	//const auto& Last = Splines.Pop();
	//const auto& ParamRange = Last.GetParamRange();
	//int32 FirstIdx = Splines.AddDefaulted();
	//int32 SecondIdx = Splines.AddDefaulted();

	//auto& First = Splines[FirstIdx];
	//auto& Second = Splines[SecondIdx];

	//TArray<FVector4> Poss;
	//TArray<double> Params;
	//Last.GetCtrlPoints(Poss);
	////Last.GetKnotIntervals(Params);

	////Last.Split(First, Second, Params.Num() > 0 ? Params[Params.Num() > 2 ? 2 : Params.Num() - 1] : 0.);
	//Last.Split(First, Second, 0.5 * (ParamRange.Get<0>() + ParamRange.Get<1>()));
	//if (Second.GetCtrlPointNum() == 0) {
	//	Splines.Pop(false);
	//}
	//if (First.GetCtrlPointNum() == 0) {
	//	Splines.Pop(false);
	//}

	//ResampleCurve();
}

void AGraphTestPlayerController::AddControlPoint(const FVector& HitPoint)
{
	FVector EndPoint = HitPointToControlPoint(HitPoint);

	auto AddCtrlPointInternal = [this, &EndPoint](TWeakPtr<FSpatialSplineGraph3::FSplineType> SplinePtr) {
		TWeakPtr<FSpatialSplineGraph3::FSplineType> TargetSplinePtr = SplinePtr;

		if (Graph.HasConnection(SplinePtr, EContactType::End)) {
			TargetSplinePtr = Graph.CreateSplineBesidesExisted(SplinePtr, EContactType::End, 1);
		}
		auto& Spline = *TargetSplinePtr.Pin().Get();

		switch (Spline.GetType()) {
		case ESplineType::BezierString:
		{
			static_cast<FSpatialBezierString3&>(Spline).AddPointAtLast(EndPoint);
			static_cast<FSpatialBezierString3&>(Spline).LastNode()->GetValue().Continuity = NewPointContinuityInit;
		}
		break;
		case ESplineType::ClampedBSpline:
		{
			static_cast<FSpatialBSpline3&>(Spline).AddPointAtLast(EndPoint);
		}
		break;
		}
	};

	auto InsertCtrlPointInternal = [this, &EndPoint](TWeakPtr<FSpatialSplineGraph3::FSplineType> SplinePtr) -> bool {
		TWeakPtr<FSpatialSplineGraph3::FSplineType> TargetSplinePtr = SplinePtr;

		double Param = -1;
		bool bFind = false;
		auto& Spline = *SplinePtr.Pin().Get();
		if (Spline.FindParamByPosition(Param, EndPoint, PointDistSqr)) {
			//UE_LOG(LogGraphTest, Warning, TEXT("Param = %.6lf"), Param);

			switch (Spline.GetType()) {
			case ESplineType::BezierString:
			{
				FSpatialBezierString3::FPointNode* NewNode = static_cast<FSpatialBezierString3&>(Spline).AddPointWithParamWithoutChangingShape(Param);
				if (NewNode) {
					NewNode->GetValue().Continuity = NewPointContinuityInit;
				}
			}
			break;
			case ESplineType::ClampedBSpline:
			{
				static_cast<FSpatialBSpline3&>(Spline).AddPointWithParamWithoutChangingShape(Param);
			}
			break;
			}
			bFind = true;
		}
		return bFind;
	};

	if (SelectedSpline.IsValid()) {
		InsertCtrlPointInternal(SelectedSpline);
		AddCtrlPointInternal(SelectedSpline);
	}
	else {
		TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
		Graph.GetSplines(Splines);
		bool bFind = false;
		for (auto& SplinePtr : Splines) {
			if (!SplinePtr.IsValid()) {
				continue;
			}
			if (InsertCtrlPointInternal(SplinePtr)) {
				bFind = true;
				break;
			}
		}

		if (!bFind) {
			if (!Splines.Last().IsValid()) {
				return;
			}
			AddCtrlPointInternal(Splines.Last());
		}
	}

	ResampleCurve();
}

void AGraphTestPlayerController::ClearCanvasImpl()
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

	NearestNodeRaw = nullptr;
	SelectedNodeRaw = nullptr;
	NearestSpline = nullptr;
	SelectedSpline = nullptr;
	NearestPoint.Reset();

	Graph.Empty();
	Graph.AddDefaulted(NewSplineType);
}

void AGraphTestPlayerController::ResampleCurve()
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

	UE_LOG(LogGraphTest, Warning, TEXT("Graph Num = %d"),
		Graph.Num());

	int32 CurLayer = 0;

	TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
	TArray<FSpatialBezierString3*> BezierStrings;
	TArray<FSpatialBSpline3*> BSplines;
	Graph.GetSplines(Splines);
	for (auto& SplinePtr : Splines) {
		if (SplinePtr.IsValid()) {
			FSpatialSplineGraph3::FSplineType* SplinePtrRaw = SplinePtr.Pin().Get();
			switch (SplinePtrRaw->GetType()) {
			case ESplineType::BezierString:
				BezierStrings.Add(static_cast<FSpatialBezierString3*>(SplinePtrRaw));
				break;
			case ESplineType::ClampedBSpline:
				BSplines.Add(static_cast<FSpatialBSpline3*>(SplinePtrRaw));
				break;
			}
		}
	}

	CurLayer = ResampleBSpline(BSplines, CurLayer);
	CurLayer = ResampleBezierString(BezierStrings, CurLayer);
}

int32 AGraphTestPlayerController::ResampleBSpline(const TArray<FSpatialBSpline3*>& Splines, int32 FirstLineLayer)
{

	int32 SplineLayer = FirstLineLayer;

	for (int32 i = 0; i < Splines.Num(); ++i) {
		UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d] CtrlPointNum = %d, KnotNum = %d"),
			i, Splines[i]->GetCtrlPointNum(), Splines[i]->GetKnotNum());

		TArray<FVector4> SpCtrlPoints; TArray<double> SpParams;
		Splines[i]->GetCtrlPoints(SpCtrlPoints);
		Splines[i]->GetKnotIntervals(SpParams);

		if (Splines[i]->GetCtrlPointNum() < 2) {
			if (Splines[i]->GetCtrlPointNum() > 0
				//&& Splines[i] == static_cast<FSpatialBSpline3*>(SelectedSpline.Pin().Get())
				) {
				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(Splines[i]->FirstNode()->GetValue().Pos)));
			}
			continue;
		}
		const auto& ParamRange = Splines[i]->GetParamRange();


		if (bDisplayControlPoint) {
			if (Splines[i] == static_cast<FSpatialBSpline3*>(SelectedSpline.Pin().Get())) {
				for (const auto& PH : SpCtrlPoints) {
					Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
					Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
				}
				Canvas2D->DrawLines(SplineLayer);
				++SplineLayer;
			}

			for (const auto& T : SpParams) {
				const auto& P = Splines[i]->GetPosition(T);
				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(P));
			}
		}

		for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
			UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].Points[%d] = <%s>"),
				i, j, *SpCtrlPoints[j].ToString());
		}
		for (int32 j = 0; j < SpParams.Num(); ++j) {
			UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].Knots[%d] = <%s, t = %.6lf>"),
				i, j, *Splines[i]->GetPosition(SpParams[j]).ToString(), SpParams[j]);
			if (bDisplaySmallTangent) {
				UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].Tangents[%d] = <%s, size = %.6lf>"),
					i, j, *Splines[i]->GetTangent(SpParams[j]).ToString(), Splines[i]->GetTangent(SpParams[j]).Size());
			}
			if (bDisplaySmallCurvature) {
				UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].PrincipalCurvatures[%d] = <%.6lf>"),
					i, j, Splines[i]->GetPrincipalCurvature(SpParams[j], 0));
			}
		}

		int32 SegNumDbl = FMath::CeilToDouble((ParamRange.Get<1>() - ParamRange.Get<0>()) / SamplePointDT);
		//for (double T = ParamRange.Get<0>(); T <= ParamRange.Get<1>(); T += SamplePointDT) {
		for (int32 Cnt = 0; Cnt <= SegNumDbl; ++Cnt) {
			double T = ParamRange.Get<0>() + (ParamRange.Get<1>() - ParamRange.Get<0>()) * static_cast<double>(Cnt) / static_cast<double>(SegNumDbl);
			FVector LinePoint = ControlPointToHitPoint(Splines[i]->GetPosition(T));
			Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);

			int32 AdditionalLayer = 0;
			if (bDisplaySmallTangent) {
				++AdditionalLayer;
				//FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T).GetSafeNormal() * 100.);
				FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T) * 1. / 3.);
				Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(TangentPoint);
			}
			if (bDisplaySmallCurvature) {
				++AdditionalLayer;
				Canvas2D->ToCanvasPoint(FVector2D(Splines[i]->GetTangent(T)).GetRotated(90.));
				FVector CurvaturePoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T).GetSafeNormal() * 1000.).RotateAngleAxis(90., FVector::BackwardVector) * Splines[i]->GetPrincipalCurvature(T, 0);
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

int32 AGraphTestPlayerController::ResampleBezierString(const TArray<FSpatialBezierString3*>& Splines, int32 FirstLineLayer)
{

	int32 SplineLayer = FirstLineLayer;

	for (int32 i = 0; i < Splines.Num(); ++i) {
		UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d] CtrlPointNum = %d"),
			i, Splines[i]->GetCtrlPointNum());

		TArray<FVector4> SpCtrlPoints;
		TArray<FVector4> SpCtrlPointsPrev;
		TArray<FVector4> SpCtrlPointsNext;
		TArray<double> SpParams;
		Splines[i]->GetCtrlPoints(SpCtrlPoints);
		Splines[i]->GetCtrlPointsPrev(SpCtrlPointsPrev);
		Splines[i]->GetCtrlPointsNext(SpCtrlPointsNext);
		Splines[i]->GetCtrlParams(SpParams);

		if (Splines[i]->GetCtrlPointNum() < 2) {
			//Canvas2D->DrawPoints(i);
			if (Splines[i]->GetCtrlPointNum() > 0) {
				//if (SelectedSpline.Pin().Get() == Splines[i]) {
					Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(Splines[i]->FirstNode()->GetValue().Pos)));
				//}
			}
			continue;
		}
		const auto& ParamRange = Splines[i]->GetParamRange();


		if (bDisplayControlPoint) {
			if (SelectedSpline.Pin().Get() == Splines[i]) 
			{
				for (const auto& PH : SpCtrlPoints) {
					Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
				}
				for (const auto& PH : SpCtrlPointsPrev) {
					Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
				}
				for (const auto& PH : SpCtrlPointsNext) {
					Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
				}
			}
			//for (const auto& T : SpParams) {
			//	const auto& P = Splines[i].GetPosition(T);
			//	Canvas2D->DisplayPoints[i].Array.Add(ControlPointToHitPoint(P));
			//}
		}

		for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
			UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].Points[%d] = <%s>, %lf"),
				i, j, *SpCtrlPoints[j].ToString(), SpParams[j]);
		}
		//for (int32 j = 0; j < SpParams.Num(); ++j) {
		//	UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].Knots[%d] = <%s, t = %.6lf>"),
		//		i, j, *Splines[i].GetPosition(SpParams[j]).ToString(), SpParams[j]);
		//	if (bDisplaySmallTangent) {
		//		UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].Tangents[%d] = <%s, size = %.6lf>"),
		//			i, j, *Splines[i].GetTangent(SpParams[j]).ToString(), Splines[i].GetTangent(SpParams[j]).Size());
		//	}
		//	if (bDisplaySmallCurvature) {
		//		UE_LOG(LogGraphTest, Warning, TEXT("Splines[%d].PrincipalCurvatures[%d] = <%.6lf>"),
		//			i, j, Splines[i].GetPrincipalCurvature(SpParams[j], 0));
		//	}
		//}

		int32 SegNumDbl = FMath::CeilToDouble((ParamRange.Get<1>() - ParamRange.Get<0>()) / SamplePointDT);
		//for (double T = ParamRange.Get<0>(); T <= ParamRange.Get<1>(); T += SamplePointDT) {
		for (int32 Cnt = 0; Cnt <= SegNumDbl; ++Cnt) {
			double T = ParamRange.Get<0>() + (ParamRange.Get<1>() - ParamRange.Get<0>()) * static_cast<double>(Cnt) / static_cast<double>(SegNumDbl);
			FVector LinePoint = ControlPointToHitPoint(Splines[i]->GetPosition(T));
			Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);

			int32 AdditionalLayer = 0;
			if (bDisplaySmallTangent) {
				++AdditionalLayer;
				//FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i].GetTangent(T).GetSafeNormal() * 100.);
				FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T) * 1. / 3.);
				Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(TangentPoint);
			}
			if (bDisplaySmallCurvature) {
				++AdditionalLayer;
				Canvas2D->ToCanvasPoint(FVector2D(Splines[i]->GetTangent(T)).GetRotated(90.));
				FVector CurvaturePoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T).GetSafeNormal() * 1000.).RotateAngleAxis(90., FVector::BackwardVector) * Splines[i]->GetPrincipalCurvature(T, 0);
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

void AGraphTestPlayerController::PressLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	bPressedLeftMouseButton = true;
}

void AGraphTestPlayerController::ReleaseLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	bPressedLeftMouseButton = false;
	if (SelectedNodeRaw) {
		SelectedNodeRaw = nullptr;
	}
	if (NearestNodeRaw) {
		SelectedNodeRaw = NearestNodeRaw;
	}

	if (SelectedSpline.IsValid() && !SelectedNodeRaw) {
		SelectedSpline = nullptr;
	}
	if (NearestSpline.IsValid()) {
		SelectedSpline = NearestSpline;
	}
	ResampleCurve();
}

void AGraphTestPlayerController::AddControlPointEvent(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
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

void AGraphTestPlayerController::AddNewSplineAfterSelectedSplineEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	if (SelectedSpline.IsValid())
	{
		Graph.CreateSplineBesidesExisted(SelectedSpline, EContactType::End, 1);
	}
	else {
		TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
		Graph.GetSplines(Splines);
		Graph.CreateSplineBesidesExisted(Splines.Last(), EContactType::End, 1);
	}

	ResampleCurve();
}

void AGraphTestPlayerController::ClearCanvasEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	ClearCanvas();
}

void AGraphTestPlayerController::RemakeBezierC2Event(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	RemakeBezierC2();
}

void AGraphTestPlayerController::FlipDisplayControlPointEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplayControlPoint();
}

void AGraphTestPlayerController::FlipDisplaySmallTangentEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallTangent();
}

void AGraphTestPlayerController::FlipDisplaySmallCurvatureEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallCurvature();
}

void AGraphTestPlayerController::SplitSplineAtCenterEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	SplitSplineAtCenter();
}

void AGraphTestPlayerController::FlipSelectedSplineTypeEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	if (SelectedSpline.IsValid()) {
		if (SelectedSpline.Pin().Get()->GetType() == ESplineType::ClampedBSpline) {
			Graph.ChangeSplineType(SelectedSpline, ESplineType::BezierString);
			ResampleCurve();
		}
	}
	//NewSplineType = (NewSplineType == ESplineType::ClampedBSpline) ? ESplineType::BezierString : ESplineType::ClampedBSpline;
}

void AGraphTestPlayerController::ReverseSelectedSplineTypeEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	if (SelectedSpline.IsValid()) {
		UE_LOG(LogGraphTest, Warning, TEXT("Reverse Spline"));
		Graph.ReverseSpline(SelectedSpline);
		ResampleCurve();
	}
}

FVector AGraphTestPlayerController::ControlPointToHitPoint(const FVector& P)
{
	return Canvas2D->ToCanvasPoint(FVector2D(P));
}

FVector AGraphTestPlayerController::HitPointToControlPoint(const FVector& P)
{
	static constexpr double Weight = 1.;
	return FVector(Canvas2D->FromCanvasPoint(P) * Weight, Weight);
}
