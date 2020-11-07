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

	return CurDistSqr.IsSet();
}
