// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Curves/BezierCurve.h"
#include "Eigen/LU"
//#include "Eigen/Dense"

// Bezier Operations on Degree 3
template<int32 Dim>
class TBezierOperationsDegree3
{
public:
	static void ConnectFromCurveToPointC2(TBezierCurve<Dim, 3>& OutCurve, const TBezierCurve<Dim, 3>& InCurve, const TVectorX<Dim>& EndPoint);

	static void ConnectFromCurveToCurveC2(TArray<TBezierCurve<Dim, 3> >& OutCurves, const TBezierCurve<Dim, 3>& InFirst, const TBezierCurve<Dim, 3>& InSecond);

	static void ConnectFromCurveToCurveC1(TArray<TBezierCurve<Dim, 3> >& OutCurves, const TBezierCurve<Dim, 3>& InFirst, const TBezierCurve<Dim, 3>& InSecond);

};

#include "BezierOperations.inl"
