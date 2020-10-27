// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "LinearAlgebraUtils.h"

constexpr int32 GaussLegendreN = 5;
constexpr int32 NewtonIteration = 10;

// Gauss-Legendre integrator. Currently only for n = 5.
template<int32 N = GaussLegendreN>
class TGaussLegendre;

template<>
class TGaussLegendre<GaussLegendreN>
{
public:
	TGaussLegendre(const TFunction<double(double)>& InGetValue, double InA = 0., double InB = 1.)
		: A(InA), B(InB)
	{
		GetValue = InGetValue;
		GetIntegration = [this](double T) -> double {
			double Result = 0.;
			double Diff = 0.5 * (T - A), Sum = 0.5 * (T + A);
			for (int32 i = 0; i < GaussLegendreN; ++i) {
				Result += Weights[i] * GetValue(Diff * Abscissa[i] + Sum);
			}
			return Result * Diff;
		};
	}

	double Integrate(double T)
	{
			return GetIntegration(T);
	}

	double SolveFromIntegration(double S, int32 Iteration = NewtonIteration) 
	{
		double EndIntegration = GetIntegration(B);
		if (FMath::IsNearlyZero(EndIntegration)) {
			return A;
		}
		double NormalRoot = S / EndIntegration;
		double Root = A*(1-NormalRoot) + B*NormalRoot;
		while (Iteration--) {
			double Value = GetValue(Root), Integration = GetIntegration(Root);
			if (FMath::IsNearlyZero(Value)) {
				return Root;
			}
			Root -= (Integration - S) / Value;
		}
		return Root;
	}

private:
	double A, B;
	TFunction<double(double)> GetValue, GetIntegration;
	static double Weights[GaussLegendreN], Abscissa[GaussLegendreN];
};


