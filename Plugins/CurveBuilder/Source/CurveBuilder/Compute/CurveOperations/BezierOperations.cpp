// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "BezierOperations.h"
THIRD_PARTY_INCLUDES_START
#include "Eigen/LU"
//#include "Eigen/Dense"
THIRD_PARTY_INCLUDES_END

namespace InternalBezier3EquationSolver
{
	template<int32 Dim = 3>
	void SolveEquationWith1stDerivative(TArray<TVectorX<Dim+1>>& OutCurvePoints, const TArray<TVectorX<Dim+1>>& InPoints, TVectorX<Dim+1> Start1stDerivative, TVectorX<Dim+1> End1stDerivative, int32 CurveNum);

	template<int32 Dim = 3>
	void SolveEquationWith2ndDerivative(TArray<TVectorX<Dim+1>>& OutCurvePoints, const TArray<TVectorX<Dim+1>>& InPoints, TVectorX<Dim+1> Start2ndDerivative, TVectorX<Dim+1> End2ndDerivative, int32 CurveNum);

};

template<int32 Dim>
void InternalBezier3EquationSolver::SolveEquationWith1stDerivative(TArray<TVectorX<Dim+1> >& OutCurvePoints, const TArray<TVectorX<Dim+1>>& InPoints, TVectorX<Dim+1> Start1stDerivative, TVectorX<Dim+1> End1stDerivative, int32 CurveNum)
{
	int32 MatrixDim = CurveNum * 4;
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
}

template<int32 Dim>
void InternalBezier3EquationSolver::SolveEquationWith2ndDerivative(TArray<TVectorX<Dim+1>>& OutCurvePoints, const TArray<TVectorX<Dim+1>>& InPoints, TVectorX<Dim+1> Start2ndDerivative, TVectorX<Dim+1> End2ndDerivative, int32 CurveNum)
{
	int32 MatrixDim = CurveNum * 4;
	OutCurvePoints.SetNum(MatrixDim);

	for (int32 c = 0; c < Dim; ++c) {
		Eigen::MatrixXd A(MatrixDim, MatrixDim);
		Eigen::VectorXd X(MatrixDim), B(MatrixDim);
		A.setZero();
		B.setZero();
		X.setZero();

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
			A(ColumnIndex, i * 4 + 4) = -1.;
			A(ColumnIndex, i * 4 + 5) = 1.;
			B(ColumnIndex) = 0.;
			++ColumnIndex;
		}

		// 3. C2 continuity (CurveDim - 1 equations)
		for (int32 i = 0; i < CurveNum - 1; ++i) {
			A(ColumnIndex, i * 4 + 1) = 1.;
			A(ColumnIndex, i * 4 + 2) = -2.;
			A(ColumnIndex, i * 4 + 3) = 1.;
			A(ColumnIndex, i * 4 + 4) = -1.;
			A(ColumnIndex, i * 4 + 5) = 2.;
			A(ColumnIndex, i * 4 + 6) = -1.;
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
}

void Bezier3EquationSolver::SolveEquationWith1stDerivative(TArray<TVectorX<4>>& OutCurvePoints, const TArray<TVectorX<4>>& InPoints, TVectorX<4> Start1stDerivative, TVectorX<4> End1stDerivative, int32 CurveNum)
{
	InternalBezier3EquationSolver::SolveEquationWith1stDerivative<3>(OutCurvePoints, InPoints, Start1stDerivative, End1stDerivative, CurveNum);
}

void Bezier3EquationSolver::SolveEquationWith2ndDerivative(TArray<TVectorX<4>>& OutCurvePoints, const TArray<TVectorX<4>>& InPoints, TVectorX<4> Start2ndDerivative, TVectorX<4> End2ndDerivative, int32 CurveNum)
{
	InternalBezier3EquationSolver::SolveEquationWith2ndDerivative<3>(OutCurvePoints, InPoints, Start2ndDerivative, End2ndDerivative, CurveNum);
}

void Bezier3EquationSolver::SolveEquationWith1stDerivative(TArray<TVectorX<3>>& OutCurvePoints, const TArray<TVectorX<3>>& InPoints, TVectorX<3> Start1stDerivative, TVectorX<3> End1stDerivative, int32 CurveNum)
{
	InternalBezier3EquationSolver::SolveEquationWith1stDerivative<2>(OutCurvePoints, InPoints, Start1stDerivative, End1stDerivative, CurveNum);
}

void Bezier3EquationSolver::SolveEquationWith2ndDerivative(TArray<TVectorX<3>>& OutCurvePoints, const TArray<TVectorX<3>>& InPoints, TVectorX<3> Start2ndDerivative, TVectorX<3> End2ndDerivative, int32 CurveNum)
{
	InternalBezier3EquationSolver::SolveEquationWith2ndDerivative<2>(OutCurvePoints, InPoints, Start2ndDerivative, End2ndDerivative, CurveNum);
}

