// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "CompletedCompTestPlayerController.h"
#include "CGDemoCanvas2D.h"
#include "ProceduralMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Logging/LogMacros.h"
#include "Engine.h"
#include "Engine/World.h"
#include "CurveBuilder/RuntimeComponent/RuntimeSplineGraph.h"
#include "CurveBuilder/RuntimeComponent/RuntimeCustomSplineBaseComponent.h"
#include "CurveBuilder/RuntimeComponent/RuntimeSplinePointBaseComponent.h"

#define GetValueRef GetValue().Get

DEFINE_LOG_CATEGORY_STATIC(LogComponentTest, Warning, All)

static const double PointDistSqr = 16.0;
static const double NodeDistSqr = 100.0;

ACompletedCompTestPlayerController::ACompletedCompTestPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = APlayerCameraManager::StaticClass();
	GraphClass = ARuntimeSplineGraph::StaticClass();
}

void ACompletedCompTestPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	Canvas2D->DrawLinesPMC->SetVisibility(false);
	Canvas2D->DrawPointsPMC->SetVisibility(false);
	Canvas2D->DrawPolygonsPMC->SetVisibility(false);
	Canvas2D->BackgroundPMC->SetVisibility(false);
	GraphActor = World->SpawnActor<ARuntimeSplineGraph>(GraphClass);

	//USceneComponent* NewRootComponent = NewObject<USceneComponent>(GraphActor);
	//GraphActor->SetRootComponent(NewRootComponent);
	//GraphActor->AddInstanceComponent(NewRootComponent);
	//NewRootComponent->RegisterComponent();
	GraphActor->SetActorRotation(FRotator(0.f, 90.f, -90.f));

	MaxSamplePointsNum = FMath::CeilToInt(static_cast<double>(Canvas2D->CanvasBoxYZ.Max.Y - Canvas2D->CanvasBoxYZ.Min.Y) / SamplePointDT) + 1;
	GetSplineGraph().Empty();
	GetSplineGraph().AddDefaulted(NewSplineType);

	InputYawScale = 0.;
	InputPitchScale = 0.;
	InputRollScale = 0.;
}

void ACompletedCompTestPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);
	if (bPressedLeftMouseButton)
	{
		FVector WorldPos, WorldDir;
		float MouseX, MouseY;
		GetMousePosition(MouseX, MouseY);
		this->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldPos, WorldDir);
		float Distance = 0;
		FVector HitPoint(0, 0, 0);
		bool bHit = TraceToCanvas(Distance, HitPoint, WorldPos, WorldDir);
		UE_LOG(LogComponentTest, Warning, TEXT("Right Mouse Button Released: %s, %s. %s"),
			*WorldPos.ToCompactString(), *WorldDir.ToCompactString(), (bHit ? TEXT("true") : TEXT("false")));
		UE_LOG(LogComponentTest, Warning, TEXT("Hit Point: %s. Distance: %.3lf"),
			*HitPoint.ToCompactString(), Distance);
		if (!bHit) {
			return;
		}

		FVector EndPoint = HitPointToControlPoint(HitPoint);

		if (!bDragging)
		{

			TArray<FHitResult> HitResults;
			FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
			//Params.bIgnoreBlocks = true;
			Params.TraceTag = TEXT("ComponentTest");
			FVector LineStart = WorldPos + WorldDir * 50.f;
			FVector LineEnd = WorldPos + WorldDir * 1000.f;
			GetWorld()->LineTraceMultiByObjectType(
				HitResults, LineStart, LineEnd,
				FCollisionObjectQueryParams::InitType::AllObjects,
				Params);
			TArray<URuntimeCustomSplineBaseComponent*> CurrentSelectedSpline;
			TMap<URuntimeCustomSplineBaseComponent*, URuntimeSplinePointBaseComponent*> CurrentSelectedSplinePoints;
			for (const FHitResult& Hit : HitResults)
			{
				UPrimitiveComponent* Comp = Hit.GetComponent();
				if (Comp->IsA<URuntimeSplinePointBaseComponent>())
				{
					URuntimeSplinePointBaseComponent* HitSpPC = Cast<URuntimeSplinePointBaseComponent>(Comp);
					if (HitSpPC && HitSpPC->bSelected)
					{
						bDragging = true;
						HitSpPC->SetRelativeLocation(HitSpPC->ConvertPosition(EndPoint, ECustomSplineCoordinateType::SplineGraphLocal, ECustomSplineCoordinateType::ComponentLocal));
						//HitSpPC->MoveTo_Deprecated(EndPoint, ECustomSplineCoordinateType::SplineGraphLocal);
						break;
					}
				}
			}
		}
		else
		{		
			for (URuntimeCustomSplineBaseComponent* Comp : GraphActor->SelectedSplines)
			{
				if (IsValid(Comp->SelectedPoint))
				{
					URuntimeSplinePointBaseComponent* HitSpPC = Comp->SelectedPoint;
					HitSpPC->SetRelativeLocation(HitSpPC->ConvertPosition(EndPoint, ECustomSplineCoordinateType::SplineGraphLocal, ECustomSplineCoordinateType::ComponentLocal));
					break;
				}
			}
		}
	}

	//NearestPoint.Reset();
	//NearestSpline = nullptr;

	//if (!bPressedLeftMouseButton) {
	//	HoldingPointType.Reset();
	//}

	////Canvas2D->DisplayLines[16].Array.Empty(Canvas2D->DisplayLines[16].Array.Num());
	////Canvas2D->DisplayPoints[1].Array.Empty(Canvas2D->DisplayPoints[1].Array.Num());
	////Canvas2D->DisplayPoints[2].Array.Empty(Canvas2D->DisplayPoints[2].Array.Num());

	//float MouseX, MouseY;
	//FVector WorldPos, WorldDir;

	//if (GetMousePosition(MouseX, MouseY) && DeprojectScreenPositionToWorld(MouseX, MouseY, WorldPos, WorldDir)) {
	//	float Distance = 0;
	//	FVector HitPoint(0, 0, 0);
	//	bool bHit = TraceToCanvas(Distance, HitPoint, WorldPos, WorldDir);
	//	if (bHit) {
	//		FVector CtrlPoint = HitPointToControlPoint(HitPoint);
	//		if (GetSplineGraph().Num()) {
	//			TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
	//			GetSplineGraph().GetSplines(Splines);
	//			for (int32 i = Splines.Num() - 1; i >= 0; --i) {
	//				auto& Spline = *Splines[i].Pin().Get();

	//				double Param = -1;
	//				if (Spline.FindParamByPosition(Param, CtrlPoint, PointDistSqr)) {
	//					FVector NearestPos = Spline.GetPosition(Param);
	//					if (FVector::DistSquared(NearestPos, CtrlPoint) < PointDistSqr) {
	//						//UE_LOG(LogComponentTest, Warning, TEXT("Param = %.6lf"), Param);
	//						NearestPoint.Emplace(NearestPos);
	//						NearestSpline = Splines[i];
	//						break;
	//					}
	//				}
	//			}
	//			if (SelectedSpline.IsValid()) {
	//				auto& Spline = *SelectedSpline.Pin().Get();
	//				CurSplineType = Spline.GetType();

	//				switch (CurSplineType) {
	//				case ESplineType::BezierString:
	//				{
	//					FSpatialBezierString3& SplineBezierString = static_cast<FSpatialBezierString3&>(Spline);
	//					NearestNodeRaw = SplineBezierString.FindNodeByPosition(CtrlPoint, 0, NodeDistSqr);
	//					FSpatialBezierString3::FPointNode* NearestNode = static_cast<FSpatialBezierString3::FPointNode*>(NearestNodeRaw);
	//					FSpatialBezierString3::FPointNode* SelectedNode = static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw);

	//					if (SelectedNode) {
	//						if (bPressedLeftMouseButton) {
	//							FVector SelectedPos = TVecLib<4>::Projection(SelectedNode->GetValueRef().Pos);
	//							FVector SelectedPrevPos = TVecLib<4>::Projection(SelectedNode->GetValueRef().PrevCtrlPointPos);
	//							FVector SelectedNextPos = TVecLib<4>::Projection(SelectedNode->GetValueRef().NextCtrlPointPos);
	//							if ((HoldingPointType && HoldingPointType.GetValue() == ESelectedNodeCtrlPointType::Current) ||
	//								(!HoldingPointType && FVector::DistSquared(SelectedPos, CtrlPoint) < NodeDistSqr)) {
	//								HoldingPointType = ESelectedNodeCtrlPointType::Current;
	//								GetSplineGraph().AdjustCtrlPointPos(SelectedPos, CtrlPoint, SelectedSpline, 1, 0, 0, NodeDistSqr);
	//								//SplineBezierString.AdjustCtrlPointPos(SelectedNode, CtrlPoint, 0);
	//								ResampleCurve();
	//							}
	//							else if ((HoldingPointType && HoldingPointType.GetValue() == ESelectedNodeCtrlPointType::Next) ||
	//								(!HoldingPointType && FVector::DistSquared(SelectedNextPos, CtrlPoint) < NodeDistSqr)) {
	//								HoldingPointType = ESelectedNodeCtrlPointType::Next;
	//								GetSplineGraph().AdjustCtrlPointPos(SelectedNextPos, CtrlPoint, SelectedSpline, 1, 1, 0, NodeDistSqr);
	//								//SplineBezierString.AdjustCtrlPointTangent(SelectedNode, CtrlPoint, true, 0);
	//								Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(SelectedNode->GetValueRef().PrevCtrlPointPos)));
	//								ResampleCurve();
	//							}
	//							else if ((HoldingPointType && HoldingPointType.GetValue() == ESelectedNodeCtrlPointType::Previous) ||
	//								(!HoldingPointType && FVector::DistSquared(SelectedPrevPos, CtrlPoint) < NodeDistSqr)) {
	//								HoldingPointType = ESelectedNodeCtrlPointType::Previous;
	//								GetSplineGraph().AdjustCtrlPointPos(SelectedPrevPos, CtrlPoint, SelectedSpline, 1, -1, 0, NodeDistSqr);
	//								//SplineBezierString.AdjustCtrlPointTangent(SelectedNode, CtrlPoint, false, 0);
	//								Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(SelectedNode->GetValueRef().NextCtrlPointPos)));
	//								ResampleCurve();
	//							}
	//							URuntimeCustomSplineBaseComponent* Comp = GraphActor->GetSplineComponentBySplineWeakPtr(SelectedSpline);
	//							Comp->UpdateTransformByCtrlPoint();
	//						}
	//					}
	//				}
	//				break;
	//				case ESplineType::ClampedBSpline:
	//				{
	//					FSpatialBSpline3& SplineBSpline = static_cast<FSpatialBSpline3&>(Spline);
	//					NearestNodeRaw = SplineBSpline.FindNodeByPosition(CtrlPoint, 0, NodeDistSqr);
	//					FSpatialBSpline3::FPointNode* NearestNode = static_cast<FSpatialBSpline3::FPointNode*>(NearestNodeRaw);
	//					FSpatialBSpline3::FPointNode* SelectedNode = static_cast<FSpatialBSpline3::FPointNode*>(SelectedNodeRaw);
	//					NearestNode = SplineBSpline.FindNodeByPosition(CtrlPoint, 0, NodeDistSqr);

	//					if (SelectedNode) {
	//						if (bPressedLeftMouseButton) {
	//							FVector SelectedPos = TVecLib<4>::Projection(SelectedNode->GetValueRef().Pos);
	//							if (HoldingPointType || (!HoldingPointType && FVector::DistSquared(SelectedPos, CtrlPoint) < NodeDistSqr)) {
	//								HoldingPointType = ESelectedNodeCtrlPointType::Current;
	//								GetSplineGraph().AdjustCtrlPointPos(SelectedPos, CtrlPoint, SelectedSpline, 1, 0, 0, NodeDistSqr);
	//								//SplineBSpline.AdjustCtrlPointPos(SelectedNode, CtrlPoint, 0);
	//								ResampleCurve();
	//							}
	//							URuntimeCustomSplineBaseComponent* Comp = GraphActor->GetSplineComponentBySplineWeakPtr(SelectedSpline);
	//							Comp->UpdateTransformByCtrlPoint();
	//						}
	//					}
	//				}
	//				break;
	//				}
	//			}
	//		}


	//		if (NearestPoint) {
	//			Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(NearestPoint.GetValue()));
	//		}
	//		if (NearestNodeRaw) {
	//			switch (CurSplineType) {
	//			case ESplineType::BezierString:
	//				Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(NearestNodeRaw)->GetValueRef().Pos)));
	//				break;
	//			}
	//		}
	//	}
	//}

	//if (SelectedNodeRaw) {
	//	switch (CurSplineType) {
	//	case ESplineType::BezierString:
	//		Canvas2D->DisplayLines[16].Array.Add(ControlPointToHitPoint(
	//			TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValueRef().PrevCtrlPointPos)));
	//		Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(
	//			TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValueRef().PrevCtrlPointPos)));
	//		Canvas2D->DisplayLines[16].Array.Add(ControlPointToHitPoint(
	//			TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValueRef().Pos)));
	//		Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(
	//			TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValueRef().Pos)));
	//		Canvas2D->DisplayLines[16].Array.Add(ControlPointToHitPoint(
	//			TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValueRef().NextCtrlPointPos)));
	//		Canvas2D->DisplayPoints[1].Array.Add(ControlPointToHitPoint(
	//			TVecLib<4>::Projection(static_cast<FSpatialBezierString3::FPointNode*>(SelectedNodeRaw)->GetValueRef().NextCtrlPointPos)));
	//		break;
	//	case ESplineType::ClampedBSpline:
	//		Canvas2D->DisplayPoints[2].Array.Add(ControlPointToHitPoint(
	//			TVecLib<4>::Projection(static_cast<FSpatialBSpline3::FPointNode*>(SelectedNodeRaw)->GetValueRef().Pos)));
	//		break;
	//	}
	//}
	//if (CurSplineType == ESplineType::BezierString) {
	//	Canvas2D->DrawLines(16);
	//}
	//Canvas2D->DrawPoints(1);
	//Canvas2D->DrawPoints(2);
}

void ACompletedCompTestPlayerController::BindOnLeftMouseButtonPressed()
{
	OnLeftMouseButtonPressed.AddDynamic(this, &ACompletedCompTestPlayerController::PressLeftMouseButton);
}

void ACompletedCompTestPlayerController::BindOnLeftMouseButtonReleased()
{
	OnLeftMouseButtonReleased.AddDynamic(this, &ACompletedCompTestPlayerController::ReleaseLeftMouseButton);
}

void ACompletedCompTestPlayerController::BindOnRightMouseButtonReleased()
{
	OnRightMouseButtonReleased.AddDynamic(this, &ACompletedCompTestPlayerController::AddControlPointEvent);
}

void ACompletedCompTestPlayerController::BindOnCtrlAndKey1Released()
{
	OnCtrlAndKey1Released.AddDynamic(this, &ACompletedCompTestPlayerController::FlipDisplayControlPointEvent);
}

void ACompletedCompTestPlayerController::BindOnCtrlAndKey2Released()
{
	OnCtrlAndKey2Released.AddDynamic(this, &ACompletedCompTestPlayerController::FlipDisplaySmallTangentEvent);
}

void ACompletedCompTestPlayerController::BindOnCtrlAndKey3Released()
{
	OnCtrlAndKey3Released.AddDynamic(this, &ACompletedCompTestPlayerController::FlipDisplaySmallCurvatureEvent);
}

void ACompletedCompTestPlayerController::BindOnCtrlAndKey4Released()
{
	//OnCtrlAndKey4Released.AddDynamic(this, &ACompletedCompTestPlayerController::SplitSplineAtCenterEvent);
	OnCtrlAndKey4Released.AddDynamic(this, &ACompletedCompTestPlayerController::ReverseSelectedSplineTypeEvent);
}

void ACompletedCompTestPlayerController::BindOnCtrlAndKey5Released()
{
	//OnCtrlAndKey5Released.AddDynamic(this, &ACompletedCompTestPlayerController::RemakeBezierC2Event);
	OnCtrlAndKey5Released.AddDynamic(this, &ACompletedCompTestPlayerController::FlipSelectedSplineTypeEvent);
}

void ACompletedCompTestPlayerController::BindOnCtrlAndKey0Released()
{
	//Super::BindOnCtrlAndKey0Released();
	OnCtrlAndKey0Released.AddDynamic(this, &ACompletedCompTestPlayerController::ClearCanvasEvent);
}

void ACompletedCompTestPlayerController::BindOnEnterReleased()
{
	Super::BindOnEnterReleased();
	OnEnterReleased.AddDynamic(this, &ACompletedCompTestPlayerController::AddNewSplineAfterSelectedSplineEvent);
}

void ACompletedCompTestPlayerController::RemakeBezierC2()
{
	//if (GetSplineGraph().Num()) {
	//	TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
	//	GetSplineGraph().GetSplines(Splines);
	//	for (auto& SplinePtr : Splines) {
	//		if (!SplinePtr.IsValid()) {
	//			continue;
	//		}
	//		auto& Spline = *SplinePtr.Pin().Get();
	//		switch (Spline.GetType()) {
	//		case ESplineType::BezierString:
	//		{
	//			auto& SplineBezierString = static_cast<FSpatialBezierString3&>(Spline);
	//			SplineBezierString.RemakeC2();
	//			for (FSpatialBezierString3::FPointNode* Node = SplineBezierString.FirstNode(); Node; Node = Node->GetNextNode()) {
	//				Node->GetValueRef().Continuity = NewPointContinuityInit;
	//			}
	//		}
	//		break;
	//		}
	//	}
	//	ResampleCurve();
	//}
}

void ACompletedCompTestPlayerController::FlipDisplayControlPoint()
{
	bDisplayControlPoint = !bDisplayControlPoint;
	UE_LOG(LogComponentTest, Warning, TEXT("DisplayMiddleControlPoint = %s"), bDisplayControlPoint ? TEXT("true") : TEXT("false"));
	//ResampleCurve();
}

void ACompletedCompTestPlayerController::FlipDisplaySmallTangent()
{
	bDisplaySmallTangent = !bDisplaySmallTangent;
	UE_LOG(LogComponentTest, Warning, TEXT("DisplaySmallTangent = %s"), bDisplaySmallTangent ? TEXT("true") : TEXT("false"));
	//ResampleCurve();
}

void ACompletedCompTestPlayerController::FlipDisplaySmallCurvature()
{
	bDisplaySmallCurvature = !bDisplaySmallCurvature;
	UE_LOG(LogComponentTest, Warning, TEXT("DisplaySmallCurvature = %s"), bDisplaySmallCurvature ? TEXT("true") : TEXT("false"));
	//ResampleCurve();
}

void ACompletedCompTestPlayerController::ClearCanvas()
{
	ClearCanvasImpl();
}

void ACompletedCompTestPlayerController::OnParamsInputChanged()
{
}

void ACompletedCompTestPlayerController::SplitSplineAtCenter()
{
	//if (GetSplineGraph().Num() == 0) {
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

void ACompletedCompTestPlayerController::AddControlPoint(const FVector& HitPoint)
{
	FVector EndPoint = HitPointToControlPoint(HitPoint);

	if (IsValid(GraphActor) && !GraphActor->IsActorBeingDestroyed())
	{
		if (GraphActor->SelectedSplines.Num() > 0)
		{
			bool bSucceed = false;
			URuntimeCustomSplineBaseComponent* TargetSplineComponent = GraphActor->SelectedSplines.Last();
			if (GraphActor->CheckSplineHasConnection(TargetSplineComponent, true))
			{
				GraphActor->ExtendNewSplineAndNewPoint(EndPoint, bSucceed, TargetSplineComponent, true, ECustomSplineCoordinateType::SplineGraphLocal);
			}
			else
			{
				GraphActor->InsertPoint(EndPoint, bSucceed, TargetSplineComponent, ECustomSplineCoordinateType::SplineGraphLocal);
			}
		}
		else
		{
			GraphActor->AddEndPoint(EndPoint, nullptr, true, ECustomSplineCoordinateType::SplineGraphLocal);
		}
	}

	//auto AddCtrlPointInternal = [this, &EndPoint](TWeakPtr<FSpatialSplineGraph3::FSplineType> SplinePtr) {
	//	TWeakPtr<FSpatialSplineGraph3::FSplineType> TargetSplinePtr = SplinePtr;

	//	bool bNewSpline = false;
	//	if (GetSplineGraph().HasConnection(SplinePtr, EContactType::End)) {
	//		TargetSplinePtr = GetSplineGraph().CreateSplineBesidesExisted(SplinePtr, EContactType::End, 1);
	//		CreateSplineActor(TargetSplinePtr);

	//		bNewSpline = true;
	//	}
	//	auto& Spline = *TargetSplinePtr.Pin().Get();
	//	URuntimeCustomSplineBaseComponent* Comp = GraphActor->GetSplineComponentBySplineWeakPtr(TargetSplinePtr);

	//	switch (Spline.GetType()) {
	//	case ESplineType::BezierString:
	//	{
	//		FSpatialBezierString3& BezierString3 = static_cast<FSpatialBezierString3&>(Spline);
	//		if (bNewSpline)
	//		{
	//			BezierString3.AdjustCtrlPointPos(BezierString3.LastNode(), EndPoint);
	//			for (FSpatialBezierString3::FPointNode* Node = BezierString3.FirstNode(); Node; Node = Node->GetNextNode())
	//			{
	//				Comp->AddPointInternal(Node->GetValue(), -1);
	//				Comp->AddPointInternal(Node->GetValue(), 0);
	//				Comp->AddPointInternal(Node->GetValue(), 1);
	//			}
	//		}
	//		else
	//		{
	//			BezierString3.AddPointAtLast(EndPoint);
	//			BezierString3.LastNode()->GetValueRef().Continuity = NewPointContinuityInit;
	//			Comp->AddPointInternal(BezierString3.LastNode()->GetValue(), -1);
	//			Comp->AddPointInternal(BezierString3.LastNode()->GetValue(), 0);
	//			Comp->AddPointInternal(BezierString3.LastNode()->GetValue(), 1);
	//		}
	//	}
	//	break;
	//	case ESplineType::ClampedBSpline:
	//	{
	//		FSpatialBSpline3& BSpline3 = static_cast<FSpatialBSpline3&>(Spline);
	//		if (bNewSpline)
	//		{
	//			for (FSpatialBSpline3::FPointNode* Node = BSpline3.FirstNode(); Node; Node = Node->GetNextNode())
	//			{
	//				Comp->AddPointInternal(Node->GetValue(), 0);
	//			}
	//		}
	//		BSpline3.AddPointAtLast(EndPoint);
	//		Comp->AddPointInternal(BSpline3.LastNode()->GetValue(), 0);
	//	}
	//	break;
	//	}
	//};

	//auto InsertCtrlPointInternal = [this, &EndPoint](TWeakPtr<FSpatialSplineGraph3::FSplineType> SplinePtr) -> bool {
	//	TWeakPtr<FSpatialSplineGraph3::FSplineType> TargetSplinePtr = SplinePtr;

	//	double Param = -1;
	//	bool bFind = false;
	//	auto& Spline = *SplinePtr.Pin().Get();
	//	URuntimeCustomSplineBaseComponent* Comp = GraphActor->GetSplineComponentBySplineWeakPtr(SplinePtr);

	//	if (Spline.FindParamByPosition(Param, EndPoint, PointDistSqr)) {
	//		//UE_LOG(LogComponentTest, Warning, TEXT("Param = %.6lf"), Param);

	//		switch (Spline.GetType()) {
	//		case ESplineType::BezierString:
	//		{
	//			FSpatialBezierString3::FPointNode* NewNode = static_cast<FSpatialBezierString3&>(Spline).AddPointWithParamWithoutChangingShape(Param);
	//			if (NewNode) {
	//				NewNode->GetValueRef().Continuity = NewPointContinuityInit;
	//				Comp->AddPointInternal(NewNode->GetValue(), -1);
	//				Comp->AddPointInternal(NewNode->GetValue(), 0);
	//				Comp->AddPointInternal(NewNode->GetValue(), 1);
	//			}
	//		}
	//		break;
	//		case ESplineType::ClampedBSpline:
	//		{
	//			FSpatialBSpline3::FPointNode* NewNode = static_cast<FSpatialBSpline3&>(Spline).AddPointWithParamWithoutChangingShape(Param);
	//			if (NewNode) {
	//				Comp->AddPointInternal(NewNode->GetValue(), 0);
	//			}
	//		}
	//		break;
	//		}
	//		bFind = true;
	//	}
	//	return bFind;
	//};

	//if (SelectedSpline.IsValid()) {
	//	if (!InsertCtrlPointInternal(SelectedSpline))
	//	{
	//		AddCtrlPointInternal(SelectedSpline);
	//	}
	//	URuntimeCustomSplineBaseComponent* Comp = GraphActor->GetSplineComponentBySplineWeakPtr(SelectedSpline);
	//	Comp->UpdateTransformByCtrlPoint();
	//}
	//else {
	//	TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
	//	GetSplineGraph().GetSplines(Splines);
	//	bool bFind = false;
	//	for (auto& SplinePtr : Splines) {
	//		if (!SplinePtr.IsValid()) {
	//			continue;
	//		}
	//		if (InsertCtrlPointInternal(SplinePtr)) {
	//			bFind = true;
	//			break;
	//		}
	//	}

	//	if (!bFind) {
	//		if (!Splines.Last().IsValid()) {
	//			return;
	//		}

	//		if (SplineActors.Num() == 0)
	//		{
	//			CreateSplineActor(Splines.Last());
	//		}
	//		AddCtrlPointInternal(Splines.Last());
	//	}
	//	URuntimeCustomSplineBaseComponent* Comp = GraphActor->GetSplineComponentBySplineWeakPtr(Splines.Last());
	//	Comp->UpdateTransformByCtrlPoint();
	//}

	//ResampleCurve();
}

void ACompletedCompTestPlayerController::ClearCanvasImpl()
{
	UE_LOG(LogTemp, Warning, TEXT("Clear Canvas"));
	//for (int32 Layer = 0; Layer < Canvas2D->DisplayPoints.Num(); ++Layer) {
	//	Canvas2D->DisplayPoints[Layer].Array.Empty(MaxSamplePointsNum);
	//}
	//for (int32 Layer = 0; Layer < Canvas2D->DisplayLines.Num(); ++Layer) {
	//	Canvas2D->DisplayLines[Layer].Array.Empty(MaxSamplePointsNum);
	//}
	//for (int32 Layer = 0; Layer < Canvas2D->DisplayPolygons.Num(); ++Layer) {
	//	Canvas2D->DisplayPolygons[Layer].Array.Empty(MaxSamplePointsNum);
	//}
	//Canvas2D->ClearDrawing();

	NearestNodeRaw = nullptr;
	SelectedNodeRaw = nullptr;
	NearestSpline = nullptr;
	SelectedSpline = nullptr;
	NearestPoint.Reset();

	GraphActor->ClearAllSplines();

	GetSplineGraph().Empty();
	GetSplineGraph().AddDefaulted(NewSplineType);
}

void ACompletedCompTestPlayerController::ResampleCurve()
{
	//for (int32 Layer = 0; Layer < Canvas2D->DisplayPoints.Num(); ++Layer) {
	//	Canvas2D->DisplayPoints[Layer].Array.Empty(MaxSamplePointsNum);
	//}
	//for (int32 Layer = 0; Layer < Canvas2D->DisplayLines.Num(); ++Layer) {
	//	Canvas2D->DisplayLines[Layer].Array.Empty(MaxSamplePointsNum);
	//}
	//for (int32 Layer = 0; Layer < Canvas2D->DisplayPolygons.Num(); ++Layer) {
	//	Canvas2D->DisplayPolygons[Layer].Array.Empty(MaxSamplePointsNum);
	//}
	//Canvas2D->ClearDrawing();

	//UE_LOG(LogComponentTest, Warning, TEXT("Graph Num = %d"),
	//	GetSplineGraph().Num());

	//int32 CurLayer = 0;

	//TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
	//TArray<FSpatialBezierString3*> BezierStrings;
	//TArray<FSpatialBSpline3*> BSplines;
	//GetSplineGraph().GetSplines(Splines);
	//for (auto& SplinePtr : Splines) {
	//	if (SplinePtr.IsValid()) {
	//		FSpatialSplineGraph3::FSplineType* SplinePtrRaw = SplinePtr.Pin().Get();
	//		switch (SplinePtrRaw->GetType()) {
	//		case ESplineType::BezierString:
	//			BezierStrings.Add(static_cast<FSpatialBezierString3*>(SplinePtrRaw));
	//			break;
	//		case ESplineType::ClampedBSpline:
	//			BSplines.Add(static_cast<FSpatialBSpline3*>(SplinePtrRaw));
	//			break;
	//		}
	//	}
	//}

	//CurLayer = ResampleBSpline(BSplines, CurLayer);
	//CurLayer = ResampleBezierString(BezierStrings, CurLayer);
}

void ACompletedCompTestPlayerController::CreateSplineActor(TWeakPtr<FSpatialSplineGraph3::FSplineType> SplineWeakPtr)
{
	//UWorld* World = GetWorld();
	//if (IsValid(World))
	//{
	//	AActor* NewActor = World->SpawnActor<AActor>();
	//	if (IsValid(NewActor))
	//	{
	//		USceneComponent* NewRootComponent = NewObject<USceneComponent>(NewActor);
	//		NewActor->SetRootComponent(NewRootComponent);
	//		NewActor->AddInstanceComponent(NewRootComponent);
	//		NewRootComponent->RegisterComponent();
	//		//NewActor->SetActorRotation(FRotator(0.f, 90.f, -90.f));
	//		SplineActors.Add(NewActor);
	//		URuntimeCustomSplineBaseComponent* NewComponent = NewObject<URuntimeCustomSplineBaseComponent>(NewActor);
	//		NewComponent->SplineBaseWrapperProxy = GetSplineGraph().GetSplineWrapper(SplineWeakPtr).Pin();
	//		NewComponent->AttachToComponent(NewActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	//		NewActor->AddInstanceComponent(NewComponent);
	//		NewComponent->RegisterComponent();
	//		GraphActor->VirtualAttachSplineComponent(NewComponent);
	//	}
	//}
}

FSpatialSplineGraph3& ACompletedCompTestPlayerController::GetSplineGraph()
{
	return GraphActor->SplineGraphProxy;
}

int32 ACompletedCompTestPlayerController::ResampleBSpline(const TArray<FSpatialBSpline3*>& Splines, int32 FirstLineLayer)
{

	int32 SplineLayer = FirstLineLayer;

	//for (int32 i = 0; i < Splines.Num(); ++i) {
	//	UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d] CtrlPointNum = %d, KnotNum = %d"),
	//		i, Splines[i]->GetCtrlPointNum(), Splines[i]->GetKnotNum());

	//	TArray<FVector4> SpCtrlPoints; TArray<double> SpParams;
	//	Splines[i]->GetCtrlPoints(SpCtrlPoints);
	//	Splines[i]->GetKnotIntervals(SpParams);

	//	if (Splines[i]->GetCtrlPointNum() < 2) {
	//		if (Splines[i]->GetCtrlPointNum() > 0
	//			//&& Splines[i] == static_cast<FSpatialBSpline3*>(SelectedSpline.Pin().Get())
	//			) {
	//			Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(Splines[i]->FirstNode()->GetValueRef().Pos)));
	//		}
	//		continue;
	//	}
	//	const auto& ParamRange = Splines[i]->GetParamRange();


	//	if (bDisplayControlPoint) {
	//		if (Splines[i] == static_cast<FSpatialBSpline3*>(SelectedSpline.Pin().Get())) {
	//			for (const auto& PH : SpCtrlPoints) {
	//				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
	//				Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
	//			}
	//			Canvas2D->DrawLines(SplineLayer);
	//			++SplineLayer;
	//		}

	//		for (const auto& T : SpParams) {
	//			const auto& P = Splines[i]->GetPosition(T);
	//			Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(P));
	//		}
	//	}

	//	for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
	//		UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].Points[%d] = <%s>"),
	//			i, j, *SpCtrlPoints[j].ToString());
	//	}
	//	for (int32 j = 0; j < SpParams.Num(); ++j) {
	//		UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].Knots[%d] = <%s, t = %.6lf>"),
	//			i, j, *Splines[i]->GetPosition(SpParams[j]).ToString(), SpParams[j]);
	//		if (bDisplaySmallTangent) {
	//			UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].Tangents[%d] = <%s, size = %.6lf>"),
	//				i, j, *Splines[i]->GetTangent(SpParams[j]).ToString(), Splines[i]->GetTangent(SpParams[j]).Size());
	//		}
	//		if (bDisplaySmallCurvature) {
	//			UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].PlanCurvatures[%d] = <%.6lf>"),
	//				i, j, Splines[i]->GetPlanCurvature(SpParams[j], 0));
	//		}
	//	}

	//	int32 SegNumDbl = FMath::CeilToDouble((ParamRange.Get<1>() - ParamRange.Get<0>()) / SamplePointDT);
	//	//for (double T = ParamRange.Get<0>(); T <= ParamRange.Get<1>(); T += SamplePointDT) {
	//	for (int32 Cnt = 0; Cnt <= SegNumDbl; ++Cnt) {
	//		double T = ParamRange.Get<0>() + (ParamRange.Get<1>() - ParamRange.Get<0>()) * static_cast<double>(Cnt) / static_cast<double>(SegNumDbl);
	//		FVector LinePoint = ControlPointToHitPoint(Splines[i]->GetPosition(T));
	//		Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);

	//		int32 AdditionalLayer = 0;
	//		if (bDisplaySmallTangent) {
	//			++AdditionalLayer;
	//			//FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T).GetSafeNormal() * 100.);
	//			FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T) * 1. / 3.);
	//			Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(TangentPoint);
	//		}
	//		if (bDisplaySmallCurvature) {
	//			++AdditionalLayer;
	//			Canvas2D->ToCanvasPoint(FVector2D(Splines[i]->GetTangent(T)).GetRotated(90.));
	//			FVector CurvaturePoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T).GetSafeNormal() * 1000.).RotateAngleAxis(90., FVector::BackwardVector) * Splines[i]->GetPlanCurvature(T, 0);
	//			Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(CurvaturePoint);
	//		}
	//	}
	//	Canvas2D->DrawLines(SplineLayer);
	//	++SplineLayer;
	//	if (bDisplaySmallTangent) {
	//		Canvas2D->DrawLines(SplineLayer);
	//		++SplineLayer;
	//	}
	//	if (bDisplaySmallCurvature) {
	//		Canvas2D->DrawLines(SplineLayer);
	//		++SplineLayer;
	//	}

	//}
	//Canvas2D->DrawPoints(0);
	return SplineLayer;
}

int32 ACompletedCompTestPlayerController::ResampleBezierString(const TArray<FSpatialBezierString3*>& Splines, int32 FirstLineLayer)
{

	int32 SplineLayer = FirstLineLayer;

	//for (int32 i = 0; i < Splines.Num(); ++i) {
	//	UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d] CtrlPointNum = %d"),
	//		i, Splines[i]->GetCtrlPointNum());

	//	TArray<FVector4> SpCtrlPoints;
	//	TArray<FVector4> SpCtrlPointsPrev;
	//	TArray<FVector4> SpCtrlPointsNext;
	//	TArray<double> SpParams;
	//	Splines[i]->GetCtrlPoints(SpCtrlPoints);
	//	Splines[i]->GetCtrlPointsPrev(SpCtrlPointsPrev);
	//	Splines[i]->GetCtrlPointsNext(SpCtrlPointsNext);
	//	Splines[i]->GetCtrlParams(SpParams);

	//	if (Splines[i]->GetCtrlPointNum() < 2) {
	//		//Canvas2D->DrawPoints(i);
	//		if (Splines[i]->GetCtrlPointNum() > 0) {
	//			//if (SelectedSpline.Pin().Get() == Splines[i]) {
	//				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(Splines[i]->FirstNode()->GetValueRef().Pos)));
	//			//}
	//		}
	//		continue;
	//	}
	//	const auto& ParamRange = Splines[i]->GetParamRange();


	//	if (bDisplayControlPoint) {
	//		if (SelectedSpline.Pin().Get() == Splines[i]) 
	//		{
	//			for (const auto& PH : SpCtrlPoints) {
	//				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
	//			}
	//			for (const auto& PH : SpCtrlPointsPrev) {
	//				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
	//			}
	//			for (const auto& PH : SpCtrlPointsNext) {
	//				Canvas2D->DisplayPoints[0].Array.Add(ControlPointToHitPoint(TVecLib<4>::Projection(PH)));
	//			}
	//		}
	//		//for (const auto& T : SpParams) {
	//		//	const auto& P = Splines[i].GetPosition(T);
	//		//	Canvas2D->DisplayPoints[i].Array.Add(ControlPointToHitPoint(P));
	//		//}
	//	}

	//	for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
	//		UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].Points[%d] = <%s>, %lf"),
	//			i, j, *SpCtrlPoints[j].ToString(), SpParams[j]);
	//	}
	//	//for (int32 j = 0; j < SpParams.Num(); ++j) {
	//	//	UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].Knots[%d] = <%s, t = %.6lf>"),
	//	//		i, j, *Splines[i].GetPosition(SpParams[j]).ToString(), SpParams[j]);
	//	//	if (bDisplaySmallTangent) {
	//	//		UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].Tangents[%d] = <%s, size = %.6lf>"),
	//	//			i, j, *Splines[i].GetTangent(SpParams[j]).ToString(), Splines[i].GetTangent(SpParams[j]).Size());
	//	//	}
	//	//	if (bDisplaySmallCurvature) {
	//	//		UE_LOG(LogComponentTest, Warning, TEXT("Splines[%d].PlanCurvatures[%d] = <%.6lf>"),
	//	//			i, j, Splines[i].GetPlanCurvature(SpParams[j], 0));
	//	//	}
	//	//}

	//	int32 SegNumDbl = FMath::CeilToDouble((ParamRange.Get<1>() - ParamRange.Get<0>()) / SamplePointDT);
	//	//for (double T = ParamRange.Get<0>(); T <= ParamRange.Get<1>(); T += SamplePointDT) {
	//	for (int32 Cnt = 0; Cnt <= SegNumDbl; ++Cnt) {
	//		double T = ParamRange.Get<0>() + (ParamRange.Get<1>() - ParamRange.Get<0>()) * static_cast<double>(Cnt) / static_cast<double>(SegNumDbl);
	//		FVector LinePoint = ControlPointToHitPoint(Splines[i]->GetPosition(T));
	//		Canvas2D->DisplayLines[SplineLayer % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(LinePoint);

	//		int32 AdditionalLayer = 0;
	//		if (bDisplaySmallTangent) {
	//			++AdditionalLayer;
	//			//FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i].GetTangent(T).GetSafeNormal() * 100.);
	//			FVector TangentPoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T) * 1. / 3.);
	//			Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(TangentPoint);
	//		}
	//		if (bDisplaySmallCurvature) {
	//			++AdditionalLayer;
	//			Canvas2D->ToCanvasPoint(FVector2D(Splines[i]->GetTangent(T)).GetRotated(90.));
	//			FVector CurvaturePoint = LinePoint + ControlPointToHitPoint(Splines[i]->GetTangent(T).GetSafeNormal() * 1000.).RotateAngleAxis(90., FVector::BackwardVector) * Splines[i]->GetPlanCurvature(T, 0);
	//			Canvas2D->DisplayLines[(SplineLayer + AdditionalLayer) % Canvas2D->LineLayerConfig.MaxLayerCount].Array.Add(CurvaturePoint);
	//		}
	//	}
	//	Canvas2D->DrawLines(SplineLayer);
	//	++SplineLayer;
	//	if (bDisplaySmallTangent) {
	//		Canvas2D->DrawLines(SplineLayer);
	//		++SplineLayer;
	//	}
	//	if (bDisplaySmallCurvature) {
	//		Canvas2D->DrawLines(SplineLayer);
	//		++SplineLayer;
	//	}

	//}
	//Canvas2D->DrawPoints(0);
	return SplineLayer;
}

void ACompletedCompTestPlayerController::PressLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	bPressedLeftMouseButton = true;
}

void ACompletedCompTestPlayerController::ReleaseLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	bPressedLeftMouseButton = false;
	bDragging = false;
	//if (SelectedNodeRaw) {
	//	SelectedNodeRaw = nullptr;
	//}
	//if (NearestNodeRaw) {
	//	SelectedNodeRaw = NearestNodeRaw;
	//}

	//if (SelectedSpline.IsValid() && !SelectedNodeRaw) {
	//	GraphActor->GetSplineComponentBySplineWeakPtr(SelectedSpline)->SetSelected(false);
	//	SelectedSpline = nullptr;
	//}
	//if (NearestSpline.IsValid()) {
	//	if (SelectedSpline.IsValid() && SelectedSpline != NearestSpline)
	//	{
	//		GraphActor->GetSplineComponentBySplineWeakPtr(SelectedSpline)->SetSelected(false);
	//	}
	//	SelectedSpline = NearestSpline;
	//	GraphActor->GetSplineComponentBySplineWeakPtr(SelectedSpline)->SetSelected(true);
	//}
	//ResampleCurve();

	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	TArray<FHitResult> HitResults; 
	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	//Params.bIgnoreBlocks = true;
	Params.TraceTag = TEXT("ComponentTest");
	FVector LineStart = WorldPos + WorldDir * 50.f;
	FVector LineEnd = WorldPos + WorldDir * 1000.f;
	GetWorld()->LineTraceMultiByObjectType(
		HitResults, LineStart, LineEnd,
		FCollisionObjectQueryParams::InitType::AllObjects,
		Params);
	TArray<URuntimeCustomSplineBaseComponent*> CurrentSelectedSpline;
	TMap<URuntimeCustomSplineBaseComponent*, URuntimeSplinePointBaseComponent*> CurrentSelectedSplinePoints;
	//DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Red, false, 10.f, 0, 2.f);
	for (const FHitResult& Result : HitResults)
	{
		//DrawDebugLine(GetWorld(), Result.TraceStart, Result.TraceEnd, FColor::Blue, false, 10.f, 0, 2.f);
		UE_LOG(LogComponentTest, Warning, TEXT(R"(LineTrace Hits:
{
	Actor:[%s],
	Component:[%s],
	Position:[%s],
	BoneName:[%s]
}
		)"),
			*Result.Actor->GetFName().ToString(),
			*Result.Component->GetFName().ToString(),
			*Result.Location.ToString(),
			*Result.BoneName.ToString());
		
		if (Result.Component->IsA<URuntimeCustomSplineBaseComponent>())
		{
			URuntimeCustomSplineBaseComponent* HitSplineComp = Cast<URuntimeCustomSplineBaseComponent>(Result.Component);
			HitSplineComp->SetSelected(true);
			CurrentSelectedSpline.Add(HitSplineComp);
			
			//static TMap<URuntimeCustomSplineBaseComponent*, FTimerHandle> Handles;

			//URuntimeCustomSplineBaseComponent* HitComp = Cast<URuntimeCustomSplineBaseComponent>(Result.Component);
			//HitComp->SetDrawDebugCollision(true);

			//FTimerHandle& Handle = Handles.FindOrAdd(HitComp, FTimerHandle());
			//GetWorldTimerManager().SetTimer(Handle, [HitComp, &Handle]()
			//{
			//	HitComp->SetDrawDebugCollision(false);
			//	Handle.Invalidate();
			//}, 3.f, false, 3.f);
		}
		else if (Result.Component->IsA<URuntimeSplinePointBaseComponent>())
		{
			URuntimeSplinePointBaseComponent* HitSplinePointComp = Cast<URuntimeSplinePointBaseComponent>(Result.Component);
			if (IsValid(HitSplinePointComp->ParentSpline))
			{
				CurrentSelectedSplinePoints.Add(HitSplinePointComp->ParentSpline, HitSplinePointComp);
			}
			HitSplinePointComp->SetSelected(true);
			if (IsValid(HitSplinePointComp->ParentSpline))
			{
				CurrentSelectedSpline.Add(HitSplinePointComp->ParentSpline);
			}
		}
	}
	if (IsValid(GraphActor))
	{
		for (int32 i = GraphActor->SelectedSplines.Num() - 1; i >= 0; --i)
		{
			if (!CurrentSelectedSpline.Contains(GraphActor->SelectedSplines[i]))
			{
				GraphActor->SelectedSplines[i]->SetSelected(false);
			}
		}
	}
	if (CurrentSelectedSplinePoints.Num() > 0)
	{
		for (URuntimeCustomSplineBaseComponent* Comp : GraphActor->SelectedSplines)
		{
			if (IsValid(Comp->SelectedPoint) && !CurrentSelectedSplinePoints.Contains(Comp))
			{
				Comp->SelectedPoint->SetSelected(false);
			}
		}
	}

	UE_LOG(LogComponentTest, Warning, TEXT(R"(End Hit Results)"));
}

void ACompletedCompTestPlayerController::AddControlPointEvent(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	float Distance = 0;
	FVector HitPoint(0, 0, 0);
	bool bHit = TraceToCanvas(Distance, HitPoint, WorldPos, WorldDir);
	UE_LOG(LogComponentTest, Warning, TEXT("Right Mouse Button Released: %s, %s. %s"),
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString(), (bHit ? TEXT("true") : TEXT("false")));
	UE_LOG(LogComponentTest, Warning, TEXT("Hit Point: %s. Distance: %.3lf"),
		*HitPoint.ToCompactString(), Distance);
	if (!bHit) {
		return;
	}
	AddControlPoint(HitPoint);
}

void ACompletedCompTestPlayerController::AddNewSplineAfterSelectedSplineEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	bool bSucceed = false;
	if (GraphActor->SelectedSplines.Num() > 0)
	{
		GraphActor->ExtendNewSplineWithContinuity(bSucceed, GraphActor->SelectedSplines.Last(), true, ECustomSplineCoordinateType::SplineGraphLocal);
	}

	//if (SelectedSpline.IsValid())
	//{
	//	CreateSplineActor(GetSplineGraph().CreateSplineBesidesExisted(SelectedSpline, EContactType::End, 1));
	//}
	//else {
	//	TArray<TWeakPtr<FSpatialSplineGraph3::FSplineType> > Splines;
	//	GetSplineGraph().GetSplines(Splines);
	//	CreateSplineActor(GetSplineGraph().CreateSplineBesidesExisted(Splines.Last(), EContactType::End, 1));
	//}

	//ResampleCurve();
}

void ACompletedCompTestPlayerController::ClearCanvasEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	ClearCanvas();
}

void ACompletedCompTestPlayerController::RemakeBezierC2Event(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	RemakeBezierC2();
}

void ACompletedCompTestPlayerController::FlipDisplayControlPointEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplayControlPoint();
}

void ACompletedCompTestPlayerController::FlipDisplaySmallTangentEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallTangent();
}

void ACompletedCompTestPlayerController::FlipDisplaySmallCurvatureEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	FlipDisplaySmallCurvature();
}

void ACompletedCompTestPlayerController::SplitSplineAtCenterEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	SplitSplineAtCenter();
}

void ACompletedCompTestPlayerController::FlipSelectedSplineTypeEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	if (SelectedSpline.IsValid()) {
		if (SelectedSpline.Pin().Get()->GetType() == ESplineType::ClampedBSpline) {
			GetSplineGraph().ChangeSplineType(SelectedSpline, ESplineType::BezierString);
			ResampleCurve();
		}
	}
	//NewSplineType = (NewSplineType == ESplineType::ClampedBSpline) ? ESplineType::BezierString : ESplineType::ClampedBSpline;
}

void ACompletedCompTestPlayerController::ReverseSelectedSplineTypeEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	if (SelectedSpline.IsValid()) {
		UE_LOG(LogComponentTest, Warning, TEXT("Reverse Spline"));
		GetSplineGraph().ReverseSpline(SelectedSpline);
		ResampleCurve();
	}
}

FVector ACompletedCompTestPlayerController::ControlPointToHitPoint(const FVector& P)
{
	return Canvas2D->ToCanvasPoint(FVector2D(P));
}

FVector ACompletedCompTestPlayerController::HitPointToControlPoint(const FVector& P)
{
	static constexpr double Weight = 1.;
	return FVector(Canvas2D->FromCanvasPoint(P) * Weight, Weight);
}

#undef GetValueRef
