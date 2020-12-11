// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Splines/SplineGraph.h"
#include "CGDemoPlayerController2D.h"
#include "../BezierStringTest/BezierStringTestPlayerController.h"
#include "ComponentTestPlayerController.generated.h"

using FSpatialSplineGraph3 = typename TSplineGraph<3, 3>;
using FSpatialBSpline3 = typename TClampedBSpline<3, 3>;
using FSpatialBezierString3 = typename TBezierString3<3>;

//UENUM(BlueprintType)
//enum class ESelectedNodeCtrlPointType : uint8
//{
//	Previous,
//	Current,
//	Next,
//};
class ARuntimeSplineGraph;

UCLASS(BlueprintType, Blueprintable)
class AComponentTestPlayerController : public ACGDemoPlayerController2D
{
	GENERATED_BODY()

public:
	AComponentTestPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void Tick(float Delta) override;

	virtual void BindOnLeftMouseButtonPressed() override;
	virtual void BindOnLeftMouseButtonReleased() override;
	virtual void BindOnRightMouseButtonReleased() override;
	virtual void BindOnCtrlAndKey1Released() override;
	virtual void BindOnCtrlAndKey2Released() override;
	virtual void BindOnCtrlAndKey3Released() override;
	virtual void BindOnCtrlAndKey4Released() override;
	virtual void BindOnCtrlAndKey5Released() override;
	virtual void BindOnCtrlAndKey0Released() override;
	virtual void BindOnEnterReleased() override;

public:

	UFUNCTION(BlueprintCallable)
		void RemakeBezierC2();

	UFUNCTION(BlueprintCallable)
		void FlipDisplayControlPoint();

	UFUNCTION(BlueprintCallable)
		void FlipDisplaySmallTangent();

	UFUNCTION(BlueprintCallable)
		void FlipDisplaySmallCurvature();

	UFUNCTION(BlueprintCallable)
		void ClearCanvas();

	UFUNCTION(BlueprintCallable)
		void OnParamsInputChanged();

	UFUNCTION(BlueprintCallable)
		void SplitSplineAtCenter();

public:
	double SamplePointDT = 1. / 128.;//1. / 8.; //1. / 256.;

	int32 MaxSamplePointsNum = 0;

	TArray<FVector> ControlPoints;

	UPROPERTY(BlueprintReadWrite)
		bool bDisplayControlPoint = true;

	UPROPERTY(BlueprintReadWrite)
		bool bDisplaySmallTangent = false;

	UPROPERTY(BlueprintReadWrite)
		bool bDisplaySmallCurvature = false;

	UPROPERTY(BlueprintReadWrite)
		TArray<AActor*> SplineActors;

	//UPROPERTY(BlueprintReadWrite)
	//FCurveBuilderTestParamsInput ParamsInput;
public:

protected:
	virtual void AddControlPoint(const FVector& HitPoint);

	virtual void ClearCanvasImpl();

	void ResampleCurve();

	void CreateSplineActor(TWeakPtr<FSpatialSplineGraph3::FSplineType> SplineWeakPtr);

	FSpatialSplineGraph3& GetSplineGraph();

private:
	int32 ResampleBSpline(const TArray<FSpatialBSpline3*>& Splines, int32 FirstLineLayer = 0);

	int32 ResampleBezierString(const TArray<FSpatialBezierString3*>& Splines, int32 FirstLineLayer = 0);

	UFUNCTION()
		void PressLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void ReleaseLeftMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void AddControlPointEvent(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void AddNewSplineAfterSelectedSplineEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void ClearCanvasEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void RemakeBezierC2Event(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipDisplayControlPointEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipDisplaySmallTangentEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipDisplaySmallCurvatureEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void SplitSplineAtCenterEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipSelectedSplineTypeEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void ReverseSelectedSplineTypeEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

public:
	//TArray<FSpatialBezierString3> Splines;
	ESplineType NewSplineType = ESplineType::ClampedBSpline;
	ESplineType CurSplineType = ESplineType::ClampedBSpline;

	TOptional<FVector> NearestPoint;

	//FSpatialBSpline3::FPointNode* NearestNode = nullptr;

	//FSpatialBSpline3::FPointNode* SelectedNode = nullptr;
	TWeakPtr<FSpatialSplineGraph3::FSplineType> NearestSpline = nullptr;
	TWeakPtr<FSpatialSplineGraph3::FSplineType> SelectedSpline = nullptr;
	void* NearestNodeRaw = nullptr;
	void* SelectedNodeRaw = nullptr;

	TOptional<ESelectedNodeCtrlPointType> HoldingPointType;

	bool bPressedLeftMouseButton = false;

	EEndPointContinuity NewPointContinuityInit = EEndPointContinuity::G1;

	TOptional<FTransform> FixedTransform;

	//FSpatialSplineGraph3 Graph;

	ARuntimeSplineGraph* GraphActor;

protected:
	FVector ControlPointToHitPoint(const FVector& P);

	FVector HitPointToControlPoint(const FVector& P);
};
