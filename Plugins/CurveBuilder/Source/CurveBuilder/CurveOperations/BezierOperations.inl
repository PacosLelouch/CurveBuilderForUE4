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
	int32 MatrixDim = CurveNum * 4;
	OutCurves.Empty(CurveNum);
	TArray<TVectorX<Dim+1> > OutCurvePoints;
	OutCurvePoints.SetNum(MatrixDim);
	
	for (int32 c = 0; c < Dim; ++c) {
		Eigen::MatrixXd A(MatrixDim, MatrixDim);
		Eigen::VectorXd X(MatrixDim), B(MatrixDim);

		// 0. Border conditions 1st (2 equations)
		A(0, 0) = 1.;
		A(0, 1) = -1.;
		B(0) = Start1stDerivative[c];

		A(1, MatrixDim - 2) = 1.;
		A(1, MatrixDim - 1) = -1.;
		B(1) = End1stDerivative[c];

		int32 ColumnIndex = 2;
		// 1. Point position & C0 continuity (2 * CurveDim equations)
		for (int32 i = 0; i < CurveNum; ++i) {
			A(ColumnIndex, i * 4) = 1.;
			B(ColumnIndex) = InPoints[i][c];
			++ColumnIndex;

			A(ColumnIndex, i * 4 + 3) = 1.;
			B(ColumnIndex) = InPoints[i + 1][c];
			++ColumnIndex;
		}

		// 2. C1 continuity (CurveDim - 1 equations)
		for (int32 i = 0; i < CurveNum - 1; ++i) {
			A(ColumnIndex, i * 4 + 2) = 1.;
			A(ColumnIndex, i * 4 + 3) = -1.;
			A(ColumnIndex, i * 4 + 4) = 1.;
			A(ColumnIndex, i * 4 + 5) = -1.;
			B(ColumnIndex) = 0.;
			++ColumnIndex;
		}

		// 3. C2 continuity (CurveDim - 1 equations)
		for (int32 i = 0; i < CurveNum - 1; ++i) {
			A(ColumnIndex, i * 4 + 1) = 1.;
			A(ColumnIndex, i * 4 + 2) = -2.;
			A(ColumnIndex, i * 4 + 3) = 1.;
			A(ColumnIndex, i * 4 + 4) = 1.;
			A(ColumnIndex, i * 4 + 5) = -2.;
			A(ColumnIndex, i * 4 + 6) = 1.;
			B(ColumnIndex) = 0.;
			++ColumnIndex;
		}

		X = A.lu().solve(B);
		for (int32 i = 0; i < MatrixDim; ++i) {
			OutCurvePoints[i][c] = X(i);
		}
	}
	for (int32 i = 0; i < MatrixDim; ++i) {
		OutCurvePoints[i][Dim] = 1.;
	}
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
	int32 MatrixDim = CurveNum * 4;
	OutCurves.Empty(CurveNum);
	TArray<TVectorX<Dim+1> > OutCurvePoints;
	OutCurvePoints.SetNum(MatrixDim);

	for (int32 c = 0; c < Dim; ++c) {
		Eigen::MatrixXd A(MatrixDim, MatrixDim);
		Eigen::VectorXd X(MatrixDim), B(MatrixDim);

		// 0. Border conditions 2nd (2 equations)
		A(0, 0) = 1.;
		A(0, 1) = -2.;
		A(0, 2) = 1.;
		B(0) = Start2ndDerivative[c];

		A(1, MatrixDim - 3) = 1.;
		A(1, MatrixDim - 2) = -2.;
		A(1, MatrixDim - 1) = 1.;
		B(1) = End2ndDerivative[c];

		int32 ColumnIndex = 2;
		// 1. Point position & C0 continuity (2 * CurveDim equations)
		for (int32 i = 0; i < CurveNum; ++i) {
			A(ColumnIndex, i * 4) = 1.;
			B(ColumnIndex) = InPoints[i][c];
			++ColumnIndex;

			A(ColumnIndex, i * 4 + 3) = 1.;
			B(ColumnIndex) = InPoints[i + 1][c];
			++ColumnIndex;
		}

		// 2. C1 continuity (CurveDim - 1 equations)
		for (int32 i = 0; i < CurveNum - 1; ++i) {
			A(ColumnIndex, i * 4 + 2) = 1.;
			A(ColumnIndex, i * 4 + 3) = -1.;
			A(ColumnIndex, i * 4 + 4) = 1.;
			A(ColumnIndex, i * 4 + 5) = -1.;
			B(ColumnIndex) = 0.;
			++ColumnIndex;
		}

		// 3. C2 continuity (CurveDim - 1 equations)
		for (int32 i = 0; i < CurveNum - 1; ++i) {
			A(ColumnIndex, i * 4 + 1) = 1.;
			A(ColumnIndex, i * 4 + 2) = -2.;
			A(ColumnIndex, i * 4 + 3) = 1.;
			A(ColumnIndex, i * 4 + 4) = 1.;
			A(ColumnIndex, i * 4 + 5) = -2.;
			A(ColumnIndex, i * 4 + 6) = 1.;
			B(ColumnIndex) = 0.;
			++ColumnIndex;
		}

		X = A.lu().solve(B);
		for (int32 i = 0; i < MatrixDim; ++i) {
			OutCurvePoints[i][c] = X(i);
		}
	}
	for (int32 i = 0; i < MatrixDim; ++i) {
		OutCurvePoints[i][Dim] = 1.;
	}
	for (int32 i = 0; i < CurveNum; ++i) {
		OutCurves.Emplace(OutCurvePoints.GetData() + (4 * i));
	}
}
