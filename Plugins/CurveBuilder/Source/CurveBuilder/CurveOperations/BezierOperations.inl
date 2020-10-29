// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "BezierOperations.h"

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

	const TVectorX<Dim>& P3 = InFirst.GetPoint(3);
	const TVectorX<Dim>& P2 = InFirst.GetPoint(2);
	const TVectorX<Dim>& P1 = InFirst.GetPoint(1);
	const TVectorX<Dim>& R0 = InSecond.GetPoint(0);
	const TVectorX<Dim>& R1 = InSecond.GetPoint(1);
	const TVectorX<Dim>& R2 = InSecond.GetPoint(2);

	// C0(0, 2)
	Out0.SetPoint(0, P3, 1.);
	Out2.SetPoint(3, R0, 1.);

	// C1(0, 2)
	Out0.SetPoint(1, P3 * 2 - P2, 1.);
	Out2.SetPoint(2, R0 * 2 - R1, 1.);

	// C2(0, 2)
	Out0.SetPoint(2, P3 * 4 - P2 * 4 + P1, 1.);
	Out2.SetPoint(1, R0 * 4 - R1 * 4 + R2, 1.);

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
