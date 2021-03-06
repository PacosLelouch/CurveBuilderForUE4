// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "CGDemoPlayerController2D.h"
#include "CurveCollection.h"
#include "CurveBuilderTestPlayerController.generated.h"

using FSpatialCurve3 = typename TSplineCurveBase<3, 3>;
using FSpatialPolynomialCurve3 = typename TPolynomialCurve<3, 3>;
using FSpatialBezierCurve3 = typename TBezierCurve<3, 3>;
using FSpatialRationalBezierCurve3 = typename TRationalBezierCurve<3, 3>;

USTRUCT(BlueprintType)
struct CURVEBUILDERTEST_API FCurveBuilderTestParamsInput
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	float CurrentWeight = 1.;

};

/**
 *
 */
UCLASS()
class CURVEBUILDERTEST_API ACurveBuilderTestPlayerController : public ACGDemoPlayerController2D
{
	GENERATED_BODY()
public:
	ACurveBuilderTestPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void BindOnRightMouseButtonReleased() override;
	virtual void BindOnCtrlAndKey1Released() override;
	virtual void BindOnCtrlAndKey2Released() override;
	virtual void BindOnCtrlAndKey3Released() override;
	virtual void BindOnCtrlAndKey4Released() override;
	virtual void BindOnCtrlAndKey5Released() override;
	virtual void BindOnCtrlAndKey0Released() override;

public:
	UFUNCTION(BlueprintCallable)
	void ChangeCurveType(ECurveType Type);

	UFUNCTION(BlueprintCallable)
	void ClearCanvas();

	UFUNCTION(BlueprintCallable)
	void OnParamsInputChanged();

public:
	double SamplePointDT = 1. / 256.;

	int32 MaxSamplePointsNum = 0;

	UPROPERTY(BlueprintReadOnly)
	ECurveType CurveType = ECurveType::Bezier;

	TArray<TPair<ECurveType, TSharedPtr<FSpatialCurve3> > > Curves;

	TArray<FVector> ControlPoints;

public:
	UPROPERTY(BlueprintReadWrite)
	FCurveBuilderTestParamsInput ParamsInput;

protected:
	virtual void AddControlPoint(const FVector& HitPoint);

	virtual void ClearCanvasImpl();

	void ResampleCurve();
private:
	UFUNCTION()
	virtual void AddControlPointEvent(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
	void ChangeToPolynomialCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
	void ChangeToBezierCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
	void ChangeToRationalBezierCurve(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
	void ChangeToAllCurves(FKey Key, EInputEvent Event, APlayerController* Ctrl);

	UFUNCTION()
	void ClearCanvasEvent(FKey Key, EInputEvent Event, APlayerController* Ctrl);
};
