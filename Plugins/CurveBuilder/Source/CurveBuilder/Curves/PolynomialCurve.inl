#include "PolynomialCurve.h"
// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

// Horner's Algorithm
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TPolynomialCurve<Dim, Degree>::GetPosition(double T)  const
{
	TVectorX<Dim> H = CtrlPoints[Degree].NonHomogeneous();
	for (int32 i = Degree - 1; i >= 0; --i) {
		H = H*T + CtrlPoints[i].NonHomogeneous();
	}
	return H;
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TPolynomialCurve<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 1)) {
		return TVectorX<Dim+1>(CtrlPoints[1] - CtrlPoints[0]).NonHomogeneous();
	}
	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree)> Hodograph;
	CreateHodograph(Hodograph);
	return Hodograph.GetPosition(T);
}

template<int32 Dim, int32 Degree>
inline double TPolynomialCurve<Dim, Degree>::GetPrincipalCurvature(double T, int32 Principal) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree)> Hodograph;
	CreateHodograph(Hodograph);

	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree-1)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVectorX<Dim>::PrincipalCurvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T), Principal);
}

template<int32 Dim, int32 Degree>
inline double TPolynomialCurve<Dim, Degree>::GetCurvature(double T) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree)> Hodograph;
	CreateHodograph(Hodograph);

	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree-1)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVectorX<Dim>::Curvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T));
}

// Itself
template<int32 Dim, int32 Degree>
inline void TPolynomialCurve<Dim, Degree>::ToPolynomialForm(TVectorX<Dim + 1>* OutPolyForm) const
{
	TVectorX<Dim+1>::CopyArray(OutPolyForm, CtrlPoints, Degree + 1);
}

template<int32 Dim, int32 Degree>
inline void TPolynomialCurve<Dim, Degree>::CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree)>& OutHodograph) const
{
	double Coefficient = 1;
	for (int32 i = 0; i < Degree; ++i) {
		Coefficient *= static_cast<double>(i + 1);
		OutHodograph.SetPoint(i, TVectorX<Dim>(CtrlPoints[i + 1] * Coefficient));
	}
}

