// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/


#include "BezierConcatPlayerController.h"
#include "CGDemoCanvas2D.h"
#include "UObject/ConstructorHelpers.h"
#include "CurveOperations/BezierOperations.h"
#include "Logging/LogMacros.h"
#include "Engine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCtrl, Warning, All);

DEFINE_LOG_CATEGORY(LogCtrl);

ABezierConcatPlayerController::ABezierConcatPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ABezierConcatPlayerController::BeginPlay()
{
	Super::BeginPlay();
	CurveType = ECurveType::Bezier;
}

void ABezierConcatPlayerController::BindOnRightMouseButtonReleased()
{
	Super::BindOnRightMouseButtonReleased();
}

void ABezierConcatPlayerController::BindOnCtrlAndKey1Released()
{
	OnCtrlAndKey1Released.AddDynamic(this, &ABezierConcatPlayerController::ChangeConcatTypeToPoint);
}

void ABezierConcatPlayerController::BindOnCtrlAndKey2Released()
{
	OnCtrlAndKey2Released.AddDynamic(this, &ABezierConcatPlayerController::ChangeConcatTypeToCurve);
}

void ABezierConcatPlayerController::BindOnCtrlAndKey3Released()
{
	// Do nothing.
}

void ABezierConcatPlayerController::BindOnCtrlAndKey4Released()
{
	// Do nothing.
}

void ABezierConcatPlayerController::BindOnCtrlAndKey5Released()
{
	// Do nothing.
}

void ABezierConcatPlayerController::BindOnCtrlAndKey0Released()
{
	Super::BindOnCtrlAndKey0Released();
}

void ABezierConcatPlayerController::ChangeConcatType(EConcatType Type)
{
	ConcatType = Type;
	ClearCanvas();
}

void ABezierConcatPlayerController::AddControlPoint(const FVector& HitPoint)
{
	auto HitPointToControlPoint = [this](const FVector& P) -> FVector{
		return FVector(Canvas2D->FromCanvasPoint(P) * ParamsInput.CurrentWeight, ParamsInput.CurrentWeight);
	};
	auto ControlPointToHitPoint = [this](const FVector& P) -> FVector {
		return Canvas2D->ToCanvasPoint(TVecLib<3>::Projection(P));
	};
	Canvas2D->DisplayPoints[Curves.Num()].Array.Add(HitPoint);
	Canvas2D->DisplayPolygons[Curves.Num()].Array.Add(HitPoint);
	FVector EndPoint = HitPointToControlPoint(HitPoint);
	ControlPoints.Add(EndPoint);
	if ((ConcatType == EConcatType::ToPoint && ControlPoints.Num() == 4) ||
		(ConcatType == EConcatType::ToCurve && (ControlPoints.Num() & 3) == 0)) { // Num % 4 == 0
		FVector* Data = ControlPoints.GetData() + (ControlPoints.Num() - 4);
		Curves.Emplace(MakeTuple(ECurveType::Bezier, TSharedPtr<FSpatialCurve3>(new FSpatialBezierCurve3(Data))));
	}
	if (ConcatType == EConcatType::ToPoint && ControlPoints.Num() > 4) {
		Canvas2D->DisplayPoints[Curves.Num()].Array.Pop();
		Canvas2D->DisplayPolygons[Curves.Num()].Array.Pop();
		const FSpatialBezierCurve3& LastCurve = *(FSpatialBezierCurve3*)Curves.Last(0).Get<1>().Get();
		FSpatialBezierCurve3& NewCurve = *(FSpatialBezierCurve3*)Curves.Emplace_GetRef(MakeTuple(ECurveType::Bezier, TSharedPtr<FSpatialCurve3>(new FSpatialBezierCurve3))).Get<1>().Get();
		int32 Layer = Curves.Num();

		TBezierOperationsDegree3<FSpatialBezierCurve3::CurveDim()>::ConnectFromCurveToPointC2(NewCurve, LastCurve, EndPoint);
		for (int32 i = 0; i <= FSpatialBezierCurve3::CurveDim(); ++i) {
			FVector DrawPoint = ControlPointToHitPoint(NewCurve.GetPoint(i));
			Canvas2D->DisplayPoints[Layer].Array.Add(DrawPoint);
			Canvas2D->DisplayPolygons[Layer].Array.Add(DrawPoint);
		}
		FString TangentStr = FString::Printf(TEXT("Tangent:\n [(%s), (%s)]"),
			*LastCurve.GetTangent(1).ToString(), *NewCurve.GetTangent(0).ToString());
		FString CurvatureStr = FString::Printf(TEXT("Curvature:\n [(%.3lf), (%.3lf)]"),
			LastCurve.GetPrincipalCurvature(1, 0), NewCurve.GetPrincipalCurvature(0, 0));
		GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor(0, 128, 255), TangentStr);
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor(0, 128, 255), CurvatureStr);
		UE_LOG(LogCtrl, Warning, TEXT("%s"), *TangentStr);
		UE_LOG(LogCtrl, Warning, TEXT("%s"), *CurvatureStr);
	}
	if (ConcatType == EConcatType::ToCurve && (ControlPoints.Num() & 7) == 0) {
		const FSpatialBezierCurve3& LastFirst = *(FSpatialBezierCurve3*)Curves.Last(1).Get<1>().Get();
		const FSpatialBezierCurve3& LastSecond = *(FSpatialBezierCurve3*)Curves.Last(0).Get<1>().Get();
		TArray<FSpatialBezierCurve3> NewCurves;
		TBezierOperationsDegree3<FSpatialBezierCurve3::CurveDim()>::ConnectFromCurveToCurveC2(NewCurves, LastFirst, LastSecond);
		for (const FSpatialBezierCurve3& NewCurve : NewCurves) {
			int32 Layer = Curves.Num();
			for (int32 i = 0; i <= FSpatialBezierCurve3::CurveDim(); ++i) {
				FVector DrawPoint = ControlPointToHitPoint(NewCurve.GetPoint(i));
				Canvas2D->DisplayPoints[Layer].Array.Add(DrawPoint);
				Canvas2D->DisplayPolygons[Layer].Array.Add(DrawPoint);
			}
			Curves.Emplace(MakeTuple(ECurveType::Bezier, TSharedPtr<FSpatialCurve3>(new FSpatialBezierCurve3(NewCurve))));
		}
		FString TangentStr = FString::Printf(TEXT("Tangent:\n [(%s), (%s)]\n [(%s), (%s)]\n [(%s), (%s)]\n [(%s), (%s)] "),
				*LastFirst.GetTangent(1).ToString(), *NewCurves[0].GetTangent(0).ToString(),
				*NewCurves[0].GetTangent(1).ToString(), *NewCurves[1].GetTangent(0).ToString(),
				*NewCurves[1].GetTangent(1).ToString(), *NewCurves[2].GetTangent(0).ToString(),
				*NewCurves[2].GetTangent(1).ToString(), *LastSecond.GetTangent(0).ToString()
			);
		FString CurvatureStr = FString::Printf(TEXT("Curvature:\n [(%.3lf), (%.3lf)]\n [(%.3lf), (%.3lf)]\n [(%.3lf), (%.3lf)]\n [(%.3lf), (%.3lf)]"),
				LastFirst.GetPrincipalCurvature(1, 0), NewCurves[0].GetPrincipalCurvature(0, 0),
				NewCurves[0].GetPrincipalCurvature(1, 0), NewCurves[1].GetPrincipalCurvature(0, 0),
				NewCurves[1].GetPrincipalCurvature(1, 0), NewCurves[2].GetPrincipalCurvature(0, 0),
				NewCurves[2].GetPrincipalCurvature(1, 0), LastSecond.GetPrincipalCurvature(0, 0)
			);
		GEngine->AddOnScreenDebugMessage(0, 5.0f, FColor(0, 128, 255), TangentStr);
		GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor(0, 128, 255), CurvatureStr);
		UE_LOG(LogCtrl, Warning, TEXT("%s"), *TangentStr);
		UE_LOG(LogCtrl, Warning, TEXT("%s"), *CurvatureStr);
	}
	ResampleCurve();
}

void ABezierConcatPlayerController::ChangeConcatTypeToPoint(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Curve To Point"));
	ChangeConcatType(EConcatType::ToPoint);
}

void ABezierConcatPlayerController::ChangeConcatTypeToCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Curve To Curve"));
	ChangeConcatType(EConcatType::ToCurve);
}
