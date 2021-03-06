// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "BezierOperations.h"

namespace Bezier3EquationSolver
{
	CURVEBUILDER_API void SolveEquationWith1stDerivative(TArray<TVectorX<4>>& OutCurvePoints, const TArray<TVectorX<4>>& InPoints, TVectorX<4> Start1stDerivative, TVectorX<4> End1stDerivative, int32 CurveNum);
	
	CURVEBUILDER_API void SolveEquationWith2ndDerivative(TArray<TVectorX<4>>& OutCurvePoints, const TArray<TVectorX<4>>& InPoints, TVectorX<4> Start2ndDerivative, TVectorX<4> End2ndDerivative, int32 CurveNum);

	CURVEBUILDER_API void SolveEquationWith1stDerivative(TArray<TVectorX<3>>& OutCurvePoints, const TArray<TVectorX<3>>& InPoints, TVectorX<3> Start1stDerivative, TVectorX<3> End1stDerivative, int32 CurveNum);

	CURVEBUILDER_API void SolveEquationWith2ndDerivative(TArray<TVectorX<3>>& OutCurvePoints, const TArray<TVectorX<3>>& InPoints, TVectorX<3> Start2ndDerivative, TVectorX<3> End2ndDerivative, int32 CurveNum);
};


template<int32 Dim>
inline void TBezierOperationsDegree3<Dim>::ConnectFromCurveToPointC2(TBezierCurve<Dim, 3>& OutCurve, const TBezierCurve<Dim, 3>& InCurve, const TVectorX<Dim>& EndPoint)
{
	const TVectorX<Dim>& P3 = InCurve.GetPoint(3);
	const TVectorX<Dim>& P2 = InCurve.GetPoint(2);
	const TVectorX<Dim>& P1 = InCurve.GetPoint(1);
	// C0: Q0 == P3
	OutCurve.SetPoint(0, P3, 1.);

	// C1: Q1 - Q0 = P3 - P2 -> Q1 == 2*P3 - P2
	OutCurve.SetPoint(1, P3 * 2 - P2, 1.);

	// C2: Q2 - 2*Q1 + Q0 == P3 - 2*P2 + P1 -> Q2 = 2*(2*P3 - P2) - 2*P2 + P1 = 4*P3 - 4*P2 + P1
	OutCurve.SetPoint(2, P3 * 4 - P2 * 4 + P1, 1.);

	// C0
	OutCurve.SetPoint(3, EndPoint, 1.);
}

template<int32 Dim>
inline void TBezierOperationsDegree3<Dim>::ConnectFromCurveToCurveC2(TArray<TBezierCurve<Dim, 3> >& OutCurves, const TBezierCurve<Dim, 3>& InFirst, const TBezierCurve<Dim, 3>& InSecond)
{
	TBezierCurve<Dim, 3>& Out0 = OutCurves.AddDefaulted_GetRef();
	TBezierCurve<Dim, 3>& Out1 = OutCurves.AddDefaulted_GetRef();
	TBezierCurve<Dim, 3>& Out2 = OutCurves.AddDefaulted_GetRef();

	const TVectorX<Dim>& A3 = InFirst.GetPoint(3);
	const TVectorX<Dim>& A2 = InFirst.GetPoint(2);
	const TVectorX<Dim>& A1 = InFirst.GetPoint(1);
	const TVectorX<Dim>& B0 = InSecond.GetPoint(0);
	const TVectorX<Dim>& B1 = InSecond.GetPoint(1);
	const TVectorX<Dim>& B2 = InSecond.GetPoint(2);

	// C0(0, 2)
	Out0.SetPoint(0, A3, 1.);
	Out2.SetPoint(3, B0, 1.);

	// C1(0, 2)
	Out0.SetPoint(1, A3 * 2 - A2, 1.);
	Out2.SetPoint(2, B0 * 2 - B1, 1.);

	// C2(0, 2)
	Out0.SetPoint(2, A3 * 4 - A2 * 4 + A1, 1.);
	Out2.SetPoint(1, B0 * 4 - B1 * 4 + B2, 1.);

	// C0(1), C1(1), C2(1), Solve 18 rank linear equation.
	// 1 * P3 - 1 * Q0                                     =  0
	// 1 * P3 + 1 * Q0 - 1 * Q1                            =  P2
	// 1 * P3 - 1 * Q0 + 2 * Q1 - 1 * Q2                   = -P1+2*P2
	//                   1 * Q1 - 2 * Q2 + 1 * Q3 - 1 * R0 = -2*R1+R2
	//                            1 * Q2 - 1 * Q3 - 1 * R0 = -R1
	//                                     1 * Q3 - 1 * R0 =  0
	// Get:
	// P3 = 1/6*(-2*P1 + 7*P2 + 2*R1 - R2)
	// Q0 = 1/6*(-2*P1 + 7*P2 + 2*R1 - R2)
	// Q1 = 1/3*(-2*P1 + 4*P2 + 2*R1 - R2)
	// Q2 = 1/3*(-P1 + 2*P2 + 4*R1 - 2*R2)
	// Q3 = 1/6*(-P1 + 2*P2 + 7*R1 - 2*R2)
	// R0 = 1/6*(-P1 + 2*P2 + 7*R1 - 2*R2)
	const TVectorX<Dim>& P3 = Out0.GetPoint(3);
	const TVectorX<Dim>& P2 = Out0.GetPoint(2);
	const TVectorX<Dim>& P1 = Out0.GetPoint(1);
	const TVectorX<Dim>& R0 = Out2.GetPoint(0);
	const TVectorX<Dim>& R1 = Out2.GetPoint(1);
	const TVectorX<Dim>& R2 = Out2.GetPoint(2);
	constexpr double OneOfSix = 1. / 6., OneOfThree = 1. / 3.;
	Out0.SetPoint(3, (-P1*2 + P2*7 + R1*2 - R2)*OneOfSix, 1.);
	Out1.SetPoint(0, (-P1*2 + P2*7 + R1*2 - R2)*OneOfSix, 1.);
	Out1.SetPoint(1, (-P1*2 + P2*4 + R1*2 - R2)*OneOfThree, 1.);
	Out1.SetPoint(2, (-P1 + P2*2 + R1*4 - R2*2)*OneOfThree, 1.);
	Out1.SetPoint(3, (-P1 + P2*2 + R1*7 - R2*2)*OneOfSix, 1.);
	Out2.SetPoint(0, (-P1 + P2*2 + R1*7 - R2*2)*OneOfSix, 1.);
}

template<int32 Dim>
inline void TBezierOperationsDegree3<Dim>::ConnectFromCurveToCurveC1(TArray<TBezierCurve<Dim, 3>>& OutCurves, const TBezierCurve<Dim, 3>& InFirst, const TBezierCurve<Dim, 3>& InSecond)
{
	TBezierCurve<Dim, 3>& Out0 = OutCurves.AddDefaulted_GetRef();

	// C0
	Out0.SetPoint(0, InFirst.GetPoint(3), 1.);
	Out0.SetPoint(3, InSecond.GetPoint(0), 1.);

	// C1
	Out0.SetPoint(1, InFirst.GetPoint(3) * 2 - InFirst.GetPoint(2), 1.);
	Out0.SetPoint(2, InSecond.GetPoint(0) * 2 - InSecond.GetPoint(1), 1.);
}

template<int32 Dim>
inline void TBezierOperationsDegree3<Dim>::InterpolationC2WithBorder1stDerivative(TArray<TBezierCurve<Dim, 3>>& OutCurves, const TArray<TVectorX<Dim+1>>& InPoints, TVectorX<Dim+1> Start1stDerivative, TVectorX<Dim+1> End1stDerivative)
{
	if (InPoints.Num() < 2) {
		return;
	}

	int32 CurveNum = InPoints.Num() - 1;
	OutCurves.Empty(CurveNum);
	TArray<TVectorX<Dim+1> > OutCurvePoints;
	
	Bezier3EquationSolver::SolveEquationWith1stDerivative(OutCurvePoints, InPoints, Start1stDerivative, End1stDerivative, CurveNum);

	for (int32 i = 0; i < CurveNum; ++i) {
		OutCurves.Emplace(OutCurvePoints.GetData() + (4 * i));
	}
}

template<int32 Dim>
inline void TBezierOperationsDegree3<Dim>::InterpolationC2WithBorder2ndDerivative(TArray<TBezierCurve<Dim, 3>>& OutCurves, const TArray<TVectorX<Dim+1>>& InPoints, TVectorX<Dim+1> Start2ndDerivative, TVectorX<Dim+1> End2ndDerivative)
{
	if (InPoints.Num() < 2) {
		return;
	}

	int32 CurveNum = InPoints.Num() - 1;
	OutCurves.Empty(CurveNum);
	TArray<TVectorX<Dim+1> > OutCurvePoints;
	
	Bezier3EquationSolver::SolveEquationWith2ndDerivative(OutCurvePoints, InPoints, Start2ndDerivative, End2ndDerivative, CurveNum);

	for (int32 i = 0; i < CurveNum; ++i) {
		OutCurves.Emplace(OutCurvePoints.GetData() + (4 * i));
	}
}
