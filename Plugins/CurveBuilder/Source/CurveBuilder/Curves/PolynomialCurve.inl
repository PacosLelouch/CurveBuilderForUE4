// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "PolynomialCurve.h"

#pragma once

// Horner's Algorithm
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TPolynomialCurve<Dim, Degree>::GetPosition(double T)  const
{
	TVectorX<Dim> H = TVecLib<Dim+1>::Projection(CtrlPoints[Degree]);
	for (int32 i = Degree - 1; i >= 0; --i) {
		H = H*T + TVecLib<Dim+1>::Projection(CtrlPoints[i]);
	}
	return H;
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TPolynomialCurve<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 1)) {
		return TVecLib<Dim+1>::Projection(CtrlPoints[1] - CtrlPoints[0]);
	}
	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);
	TVectorX<Dim> Tangent = Hodograph.GetPosition(T);
	return Tangent.IsNearlyZero() ? Hodograph.GetTangent(T) : Tangent;
}

template<int32 Dim, int32 Degree>
inline double TPolynomialCurve<Dim, Degree>::GetPlanCurvature(double T, int32 PlanIndex) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::PlanCurvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T), PlanIndex);
}

template<int32 Dim, int32 Degree>
inline double TPolynomialCurve<Dim, Degree>::GetCurvature(double T) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TPolynomialCurve<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::Curvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T));
}

// Itself
template<int32 Dim, int32 Degree>
inline void TPolynomialCurve<Dim, Degree>::ToPolynomialForm(TVectorX<Dim+1>* OutPolyForm) const
{
	TVecLib<Dim+1>::CopyArray(OutPolyForm, CtrlPoints, Degree + 1);
}

template<int32 Dim, int32 Degree>
inline void TPolynomialCurve<Dim, Degree>::CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const
{
	double Coefficient = 1;
	for (int32 i = 0; i < Degree; ++i) {
		Coefficient *= static_cast<double>(i + 1);
		OutHodograph.SetPoint(i, TVectorX<Dim>(CtrlPoints[i + 1] * Coefficient));
	}
}

template<int32 Dim, int32 Degree>
inline void TPolynomialCurve<Dim, Degree>::ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& InCurve)
{
	CtrlPoints[Degree] = TVectorX<Dim+1>();
	TVecLib<Dim+1>::Last(CtrlPoints[Degree]) = 1.;
	constexpr int32 FromDegree = CLAMP_DEGREE(Degree-1, 0);
	for (int32 i = 0; i <= FromDegree; ++i) {
		CtrlPoints[i] = InCurve.GetPointHomogeneous(i);
	}
}

