#include "SplineCurveBase.h"
// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

template<int32 Dim, int32 Degree>
inline bool TSplineCurveBase<Dim, Degree>::FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr) const
{
	auto SegDbl = static_cast<double>(Degree - 1);
	TFunction<TVectorX<Dim>(double)> GetValue = [this](double T) {
		return this->GetPosition(T);
	};
	TFunction<TVectorX<Dim>(double)> GetDerivative = [this](double T) {
		return this->GetTangent(T);
	};
	TNewton<Dim> Newton(GetValue, GetDerivative, 0., 1.);

	TOptional<double> CurDistSqr;
	for (int32 i = 0; i < Degree; ++i) {
		double InitGuess = static_cast<double>(i) / SegDbl;
		double NewParam = Newton.Solve(InPos, InitGuess, NumericalCalculationConst::NewtonOptionalClampScale);
		TVectorX<Dim> NewPos = GetPosition(NewParam);
		double NewDistSqr = TVecLib<Dim>::SizeSquared(NewPos - InPos);
		if (NewDistSqr <= ToleranceSqr) {
			if (!CurDistSqr || CurDistSqr.GetValue() > NewDistSqr) {
				CurDistSqr = NewDistSqr;
				OutParam = NewParam;
			}
		}
	}

	return CurDistSqr.IsSet() && CurDistSqr.GetValue() < ToleranceSqr;
}

template<int32 Dim, int32 Degree>
inline bool TSplineCurveBase<Dim, Degree>::FindParamsByComponentValue(TArray<double>& OutParams, double InValue, int32 InComponentIndex, double ToleranceSqr) const
{
	OutParams.Empty(Degree);
	auto SegDbl = static_cast<double>(Degree - 1);
	TFunction<double(double)> GetValue = [this, InComponentIndex](double T) {
		return TVecLib<Dim>::IndexOf(this->GetPosition(T), InComponentIndex);
	};
	TFunction<double(double)> GetDerivative = [this, InComponentIndex](double T) {
		return TVecLib<Dim>::IndexOf(this->GetTangent(T), InComponentIndex);
	};
	TNewton<1> Newton(GetValue, GetDerivative, 0., 1.);

	TOptional<double> CurDistSqr;
	for (int32 i = 0; i < Degree; ++i) {
		double InitGuess = static_cast<double>(i) / SegDbl;
		double NewParam = Newton.Solve(InValue, InitGuess, NumericalCalculationConst::NewtonOptionalClampScale);
		double NewValue = TVecLib<Dim>::IndexOf(GetPosition(NewParam), InComponentIndex);
		double NewDistSqr = FMath::Square(NewValue - InValue);
		if (NewDistSqr <= ToleranceSqr) {
			if (!CurDistSqr || CurDistSqr.GetValue() > NewDistSqr) {
				CurDistSqr = NewDistSqr;
			}
			bool bExisted = false;
			for (double ExistedParam : OutParams)
			{
				if (FMath::IsNearlyEqual(ExistedParam, NewParam, 1e-3))
				{
					bExisted = true;
					break;
				}
			}
			if (!bExisted)
			{
				OutParams.Add(NewParam);
			}
		}
	}

	return CurDistSqr.IsSet() && CurDistSqr.GetValue() < ToleranceSqr;
}
