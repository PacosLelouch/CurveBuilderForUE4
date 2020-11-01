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
	OnCtrlAndKey1Released.AddDynamic(this, &ASplineTestPlayerController::ChangeConcatTypeToPoint);
}

void ASplineTestPlayerController::BindOnCtrlAndKey2Released()
{
	OnCtrlAndKey2Released.AddDynamic(this, &ASplineTestPlayerController::ChangeConcatTypeToCurve);
}

void ASplineTestPlayerController::BindOnCtrlAndKey3Released()
{
	// Do nothing.
}

void ASplineTestPlayerController::BindOnCtrlAndKey4Released()
{
	// Do nothing.
}

void ASplineTestPlayerController::BindOnCtrlAndKey5Released()
{
	// Do nothing.
}

void ASplineTestPlayerController::BindOnCtrlAndKey0Released()
{
	Super::BindOnCtrlAndKey0Released();
}

void ASplineTestPlayerController::BindOnEnterReleased()
{
	Super::BindOnEnterReleased();
	Splines.AddDefaulted();
}

void ASplineTestPlayerController::ChangeConcatType(ESplineConcatType Type)
{
	ConcatType = Type;
	ClearCanvas();
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
	Canvas2D->DisplayPoints[Splines.Num() - 1].Array.Add(HitPoint);
	FVector EndPoint = HitPointToControlPoint(HitPoint);
	ControlPoints.Add(EndPoint);
	Splines.Last().AddPointAtLast(EndPoint);

	//if ((ConcatType == ESplineConcatType::ToPoint && ControlPoints.Num() == 4) ||
	//	(ConcatType == ESplineConcatType::ToCurve && (ControlPoints.Num() & 3) == 0)) { // Num % 4 == 0
	//	FVector* Data = ControlPoints.GetData() + (ControlPoints.Num() - 4);
	//	Curves.Emplace(MakeTuple(ECurveType::Bezier, TSharedPtr<FSpatialCurve3>(new FSpatialBezierCurve3(Data))));
	//}
	//if (ConcatType == ESplineConcatType::ToPoint && ControlPoints.Num() > 4) {
	//	Canvas2D->DisplayPoints[Curves.Num()].Array.Pop();
	//	Canvas2D->DisplayPolygons[Curves.Num()].Array.Pop();
	//	const FSpatialBezierCurve3& LastCurve = *static_cast<FSpatialBezierCurve3*>(Curves.Last(0).Get<1>().Get());
	//	FSpatialBezierCurve3& NewCurve = *static_cast<FSpatialBezierCurve3*>(Curves.Emplace_GetRef(MakeTuple(ECurveType::Bezier, TSharedPtr<FSpatialCurve3>(new FSpatialBezierCurve3))).Get<1>().Get());
	//	int32 Layer = Curves.Num();

	//	TBezierOperationsDegree3<FSpatialBezierCurve3::CurveDim()>::ConnectFromCurveToPointC2(NewCurve, LastCurve, EndPoint);
	//	for (int32 i = 0; i <= FSpatialBezierCurve3::CurveDim(); ++i) {
	//		FVector DrawPoint = ControlPointToHitPoint(NewCurve.GetPoint(i));
	//		Canvas2D->DisplayPoints[Layer].Array.Add(DrawPoint);
	//		Canvas2D->DisplayPolygons[Layer].Array.Add(DrawPoint);
	//	}
	//	FString TangentStr = FString::Printf(TEXT("Tangent:\n [(%s)->%.6f, (%s)->%.6f]"),
	//		*LastCurve.GetTangent(1).ToString(), LastCurve.GetTangent(1).Size(), *NewCurve.GetTangent(0).ToString(), NewCurve.GetTangent(0).Size());
	//	FString CurvatureStr = FString::Printf(TEXT("Curvature:\n [(%.6lf), (%.6lf)]"),
	//		LastCurve.GetPrincipalCurvature(1, 0), NewCurve.GetPrincipalCurvature(0, 0));
	//	GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor(0, 128, 255), TangentStr);
	//	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor(0, 128, 255), CurvatureStr);
	//	UE_LOG(LogSplineCtrl, Warning, TEXT("%s"), *TangentStr);
	//	UE_LOG(LogSplineCtrl, Warning, TEXT("%s"), *CurvatureStr);
	//}
	//if (ConcatType == ESplineConcatType::ToCurve && (ControlPoints.Num() & 7) == 0) {
	//	const FSpatialBezierCurve3& LastFirst = *(FSpatialBezierCurve3*)Curves.Last(1).Get<1>().Get();
	//	const FSpatialBezierCurve3& LastSecond = *(FSpatialBezierCurve3*)Curves.Last(0).Get<1>().Get();
	//	TArray<FSpatialBezierCurve3> NewCurves;
	//	TBezierOperationsDegree3<FSpatialBezierCurve3::CurveDim()>::ConnectFromCurveToCurveC2(NewCurves, LastFirst, LastSecond);
	//	for (const FSpatialBezierCurve3& NewCurve : NewCurves) {
	//		int32 Layer = Curves.Num();
	//		for (int32 i = 0; i <= FSpatialBezierCurve3::CurveDim(); ++i) {
	//			FVector DrawPoint = ControlPointToHitPoint(NewCurve.GetPoint(i));
	//			Canvas2D->DisplayPoints[Layer].Array.Add(DrawPoint);
	//			Canvas2D->DisplayPolygons[Layer].Array.Add(DrawPoint);
	//		}
	//		Curves.Emplace(MakeTuple(ECurveType::Bezier, TSharedPtr<FSpatialCurve3>(new FSpatialBezierCurve3(NewCurve))));
	//	}
	//	FString TangentStr = FString::Printf(TEXT("Tangent:\n [(%s), (%s)]\n [(%s), (%s)]\n [(%s), (%s)]\n [(%s), (%s)] "),
	//		*LastFirst.GetTangent(1).ToString(), *NewCurves[0].GetTangent(0).ToString(),
	//		*NewCurves[0].GetTangent(1).ToString(), *NewCurves[1].GetTangent(0).ToString(),
	//		*NewCurves[1].GetTangent(1).ToString(), *NewCurves[2].GetTangent(0).ToString(),
	//		*NewCurves[2].GetTangent(1).ToString(), *LastSecond.GetTangent(0).ToString()
	//	);
	//	FString CurvatureStr = FString::Printf(TEXT("Curvature:\n [(%.6lf), (%.6lf)]\n [(%.6lf), (%.6lf)]\n [(%.6lf), (%.6lf)]\n [(%.6lf), (%.6lf)]"),
	//		LastFirst.GetPrincipalCurvature(1, 0), NewCurves[0].GetPrincipalCurvature(0, 0),
	//		NewCurves[0].GetPrincipalCurvature(1, 0), NewCurves[1].GetPrincipalCurvature(0, 0),
	//		NewCurves[1].GetPrincipalCurvature(1, 0), NewCurves[2].GetPrincipalCurvature(0, 0),
	//		NewCurves[2].GetPrincipalCurvature(1, 0), LastSecond.GetPrincipalCurvature(0, 0)
	//	);
	//	GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor(0, 128, 255), TangentStr);
	//	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor(0, 128, 255), CurvatureStr);
	//	UE_LOG(LogSplineCtrl, Warning, TEXT("%s"), *TangentStr);
	//	UE_LOG(LogSplineCtrl, Warning, TEXT("%s"), *CurvatureStr);
	//}
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
	UE_LOG(LogSplineCtrl, Warning, TEXT("Splines Num = %d, CtrlPoints Num = %d"),
		Splines.Num(), ControlPoints.Num());
	for (int32 i = 0; i < Splines.Num(); ++i) {
		UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d] Num = %d"),
			i, Splines[i].GetCtrlPointNum());
		Canvas2D->DrawPoints(i);

		TArray<FVector4> SpCtrlPoints; TArray<double> SpParams;
		Splines[i].GetOpenFormPointsAndParams(SpCtrlPoints, SpParams);
		for (int32 j = 0; j < SpCtrlPoints.Num(); ++j) {
			UE_LOG(LogSplineCtrl, Warning, TEXT("Splines[%d].Points[%d] = <%s, %.6lf>"),
				i, j, *SpCtrlPoints[j].ToString(), SpParams[j]);
		}

		if (Splines[i].GetCtrlPointNum() < 2) {
			continue;
		}
		const auto& ParamRange = Splines[i].GetParamRange();
		for (double T = ParamRange.Get<0>(); T <= ParamRange.Get<1>(); T += SamplePointDT) {
			FVector LinePoint = Canvas2D->ToCanvasPoint(TVecLib<3>::Projection(Splines[i].GetPosition(T)));
			Canvas2D->DisplayLines[i].Array.Add(LinePoint);
		}
		Canvas2D->DrawLines(i);
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
