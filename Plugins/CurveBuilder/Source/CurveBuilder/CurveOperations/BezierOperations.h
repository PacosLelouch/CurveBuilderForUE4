// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Curves/BezierCurve.h"
#include "Splines/BezierString.h"
//#include "Eigen/LU"
//#include "Eigen/Dense"

// Bezier Operations on Degree 3
template<int32 Dim>
class TBezierOperationsDegree3
{
public:
	// P-Q
	static void ConnectFromCurveToPointC2(TBezierCurve<Dim, 3>& OutCurve, const TBezierCurve<Dim, 3>& InCurve, const TVectorX<Dim>& EndPoint);

	// A-P-Q-R-B
	static void ConnectFromCurveToCurveC2(TArray<TBezierCurve<Dim, 3> >& OutCurves, const TBezierCurve<Dim, 3>& InFirst, const TBezierCurve<Dim, 3>& InSecond);

	// A-P-B
	static void ConnectFromCurveToCurveC1(TArray<TBezierCurve<Dim, 3> >& OutCurves, const TBezierCurve<Dim, 3>& InFirst, const TBezierCurve<Dim, 3>& InSecond);

	//static void AdjustPointC1(...);
};

#include "BezierOperations.inl"
