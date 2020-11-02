// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "../CurveBuilderTestPlayerController.h"
#include "Splines/BSpline.h"
#include "SplineTestPlayerController.generated.h"

using FSpatialBSpline3 = typename TClampedBSpline<3, 3>;

//USTRUCT(BlueprintType)
//struct CURVEBUILDERTEST_API FCurveBuilderTestParamsInput
//{
//	GENERATED_BODY()
//public:
//	UPROPERTY(BlueprintReadWrite)
//		float CurrentWeight = 1.;
//
//};

UENUM(BlueprintType)
enum class ESplineConcatType : uint8
{
	ToPoint,
	ToCurve,
};

/**
 *
 */
UCLASS()
class CURVEBUILDERTEST_API ASplineTestPlayerController : public ACGDemoPlayerController2D
{
	GENERATED_BODY()
public:
	ASplineTestPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

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
		void ChangeConcatType(ESplineConcatType Type);

	UFUNCTION(BlueprintCallable)
		void FlipConvertToPolynomialForm();

	UFUNCTION(BlueprintCallable)
		void FlipDisplayControlPoint();

	UFUNCTION(BlueprintCallable)
		void FlipDisplaySmallTangentOfInternalKnot();

	UFUNCTION(BlueprintCallable)
		void FlipDisplaySmallCurvatureOfInternalKnot();

	UFUNCTION(BlueprintCallable)
		void ClearCanvas();

	UFUNCTION(BlueprintCallable)
		void OnParamsInputChanged();

public:
	double SamplePointDT = 1. / 8.; //1. / 256.;

	int32 MaxSamplePointsNum = 0;

	TArray<FVector> ControlPoints;

	UPROPERTY(BlueprintReadOnly)
		ESplineConcatType ConcatType = ESplineConcatType::ToPoint;

	UPROPERTY(BlueprintReadWrite)
		bool bConvertToPolynomialForm = false;

	UPROPERTY(BlueprintReadWrite)
		bool bDisplayControlPoint = false;

	UPROPERTY(BlueprintReadWrite)
		bool bDisplaySmallTangent = true;

	UPROPERTY(BlueprintReadWrite)
		bool bDisplaySmallCurvature = true;

	//UPROPERTY(BlueprintReadWrite)
	//FCurveBuilderTestParamsInput ParamsInput;
public:

protected:
	virtual void AddControlPoint(const FVector& HitPoint);

	virtual void ClearCanvasImpl();

	void ResampleCurve();

private:

	UFUNCTION()
		void AddControlPointEvent(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void AddNewSplineEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void ClearCanvasEvent(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
		void ChangeConcatTypeToPoint(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void ChangeConcatTypeToCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipConvertToPolynomialFormEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipDisplayControlPointEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipDisplaySmallTangentOfInternalKnotEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
		void FlipDisplaySmallCurvatureOfInternalKnotEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);

public:
	void TestCopy() {
		if (Splines.Num() > 0) {
			FSpatialBSpline3 NewSpline(Splines.Last());
		}
	}

public:
	TArray<FSpatialBSpline3> Splines;
};
