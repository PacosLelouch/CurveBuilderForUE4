// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "LinearAlgebraUtils.h"

namespace NumericalCalculationConst {
	constexpr int32 GaussLegendreN = 5;
	constexpr int32 NewtonIteration = 5;
	constexpr double NewtonOptionalClampScale = 0.75;
}

template<int32 Dim>
class TNewton
{
public:
	TNewton(const TFunction<TVectorX<Dim>(double)>& InGetValue,
		const TFunction<TVectorX<Dim>(double)>& InGetDerivative,
		double InA = 0., double InB = 1., int32 InIteration = NumericalCalculationConst::NewtonIteration)
		: Iteration(InIteration), A(InA), B(InB), GetValue(InGetValue), GetDerivative(InGetDerivative){}

	double Solve(const TVectorX<Dim>& TargetValue, TOptional<double> InitGuess = TOptional<double>(), TOptional<double> ClampMoveScale = TOptional<double>()) {
		int32 CurIteration = Iteration;
		TVectorX<Dim> Value = GetValue(B);
		if (TVecLib<Dim>::IsNearlyZero(TargetValue - Value)) {
			return B;
		}
		double NormalRoot = TVecLib<Dim>::SizeSquared(TargetValue) / TVecLib<Dim>::SizeSquared(Value);
		double Root = InitGuess.Get(A*(1.-NormalRoot) + B*NormalRoot);
		double LastMoveAbs = B - A;
		while (CurIteration--) {
			TVectorX<Dim> Derivative = GetDerivative(Root);
			Value = GetValue(Root);
			if (TVecLib<Dim>::IsNearlyZero(Derivative)) {
				return Root;
			}
			double Move = TVecLib<Dim>::Dot((TargetValue - Value), Derivative) / TVecLib<Dim>::SizeSquared(Derivative);
			if (ClampMoveScale) {
				Move = FMath::Clamp(Move, -LastMoveAbs * ClampMoveScale.GetValue(), LastMoveAbs * ClampMoveScale.GetValue());
				LastMoveAbs = FMath::Abs(Move);
			}
			Root += Move;
		}
		return Root;
	}
protected:
	int32 Iteration;
	double A, B;
	TFunction<TVectorX<Dim>(double)> GetValue, GetDerivative;
};

//template<>
//class TNewton<1>
//{
//public:
//	TNewton(const TFunction<double(double)>& InGetValue,
//		const TFunction<double(double)>& InGetDerivative,
//		double InA = 0., double InB = 1., int32 InIteration = NumericalCalculationConst::NewtonIteration)
//		: Iteration(InIteration), A(InA), B(InB), GetValue(InGetValue), GetDerivative(InGetDerivative) {}
//
//	double Solve(double TargetValue, TOptional<double> InitGuess = TOptional<double>(), TOptional<double> ClampMoveScale = TOptional<double>()) {
//		int32 CurIteration = Iteration;
//		double Value = GetValue(B);
//		if (FMath::IsNearlyZero(TargetValue - Value)) {
//			return B;
//		}
//		double NormalRoot = TargetValue / Value;
//		double Root = InitGuess.Get(A*(1.-NormalRoot) + B*NormalRoot);
//		double LastMoveAbs = B - A;
//		while (CurIteration--) {
//			double Derivative = GetDerivative(Root);
//			Value = GetValue(Root);
//			if (FMath::IsNearlyZero(Derivative)) {
//				return Root;
//			}
//			double Move = (TargetValue - Value) / Derivative;
//			if (ClampMoveScale) {
//				Move = FMath::Clamp(Move, -LastMoveAbs * ClampMoveScale.GetValue(), LastMoveAbs * ClampMoveScale.GetValue());
//				LastMoveAbs = FMath::Abs(Move);
//			}
//			Root += Move;
//		}
//		return Root;
//	}
//protected:
//	int32 Iteration;
//	double A, B;
//	TFunction<double(double)> GetValue, GetDerivative;
//};

// Gauss-Legendre integrator. Currently only for n = 5.
template<int32 N = NumericalCalculationConst::GaussLegendreN>
class TGaussLegendre;

template<>
class TGaussLegendre<NumericalCalculationConst::GaussLegendreN>
{
public:
	TGaussLegendre(const TFunction<double(double)>& InGetValue, double InA = 0., double InB = 1.)
		: A(InA), B(InB)
	{
		GetValue = InGetValue;
		GetIntegration = [this](double T) -> double {
			double Result = 0.;
			double Diff = 0.5 * (T - A), Sum = 0.5 * (T + A);
			for (int32 i = 0; i < NumericalCalculationConst::GaussLegendreN; ++i) {
				Result += Weights[i] * GetValue(Diff * Abscissa[i] + Sum);
			}
			return Result * Diff;
		};
	}

	double Integrate(double T)
	{
			return GetIntegration(T);
	}

	double SolveFromIntegration(double S, int32 Iteration = NumericalCalculationConst::NewtonIteration)
	{
		TNewton<1> Newton(GetIntegration, GetValue, A, B, Iteration);
		return Newton.Solve(S);
		//double EndIntegration = GetIntegration(B);
		//if (FMath::IsNearlyZero(EndIntegration)) {
		//	return A;
		//}
		//double NormalRoot = S / EndIntegration;
		//double Root = A*(1-NormalRoot) + B*NormalRoot;
		//while (Iteration--) {
		//	double Value = GetValue(Root), Integration = GetIntegration(Root);
		//	if (FMath::IsNearlyZero(Value)) {
		//		return Root;
		//	}
		//	Root -= (Integration - S) / Value;
		//}
		//return Root;
	}

protected:
	double A, B;
	TFunction<double(double)> GetValue, GetIntegration;
	CURVEBUILDER_API static double Weights[NumericalCalculationConst::GaussLegendreN];
	CURVEBUILDER_API static double Abscissa[NumericalCalculationConst::GaussLegendreN];
};

using FGaussLegendre5 = typename TGaussLegendre<5>;
