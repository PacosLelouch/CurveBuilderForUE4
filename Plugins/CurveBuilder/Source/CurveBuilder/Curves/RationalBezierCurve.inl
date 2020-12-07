// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RationalBezierCurve.h"

#pragma once

// The de Casteljau Algorithm (or Horner's Algorithm if necessary)
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetPosition(double T) const
{
	//if (Dim >= 5) {
	//	return Horner(T);
	//}
	return DeCasteljau(T);
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 1)) {
		return TVecLib<Dim+1>::Projection(CtrlPoints[1] - CtrlPoints[0]) * static_cast<double>(Degree);
	}
	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);
	TVectorX<Dim> Tangent = Hodograph.GetPosition(T);
	return Tangent.IsNearlyZero() ? Hodograph.GetTangent(T) : Tangent;
}

template<int32 Dim, int32 Degree>
inline double TRationalBezierCurve<Dim, Degree>::GetPlanCurvature(double T, int32 PlanIndex) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::PlanCurvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T), PlanIndex);
}

template<int32 Dim, int32 Degree>
inline double TRationalBezierCurve<Dim, Degree>::GetCurvature(double T) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::Curvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T));
}

// Using Taylor's Series: B(t) = Sum{ 1/n! * (d^n(B)/dt^n)(t) * t^n }
template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::ToPolynomialForm(TVectorX<Dim+1> OutPolyForm[Degree + 1]) const
{
	TVectorX<Dim+1> DTable[Degree + 1];
	TVecLib<Dim+1>::CopyArray(DTable, CtrlPoints, Degree + 1);
	double Combination = 1;
	OutPolyForm[0] = CtrlPoints[0];
	TVecLib<Dim+1>::WeightToOne(OutPolyForm[0]);
	TVecLib<Dim+1>::Last(OutPolyForm[0]) = 1.;
	for (int32 i = 1; i <= Degree; ++i) {
		for (int32 j = 0; j <= Degree - i; ++j) {
			DTable[j] = DTable[j + 1] - DTable[j];
		}
		Combination *= static_cast<double>(Degree - i + 1) / i;
		OutPolyForm[i] = DTable[0] * Combination;
		TVecLib<Dim+1>::WeightToOne(OutPolyForm[i]);
		TVecLib<Dim+1>::Last(OutPolyForm[i]) = 1.;
	}
}

// How to deal with the situation where w0 == 0.0?
template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const
{
	for (int32 i = 0; i < Degree; ++i) {
		OutHodograph.SetPoint(i, TVectorX<Dim>(CtrlPoints[i + 1] - CtrlPoints[i]) * static_cast<double>(Degree), 
			TVecLib<Dim+1>::Last(CtrlPoints[i + 1]) / TVecLib<Dim+1>::Last(CtrlPoints[i]));
	}
}

template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& InCurve)
{
	constexpr int32 FromDegree = CLAMP_DEGREE(Degree-1, 0);
	constexpr double ClampDenominator = CLAMP_DEGREE(Degree, 1);
	CtrlPoints[0] = InCurve.GetPointHomogeneous(0);
	for (int32 i = 1; i < Degree; ++i) {
		double Alpha = static_cast<double>(i) / ClampDenominator;
		CtrlPoints[i] = InCurve.GetPointHomogeneous(i - 1)*Alpha + InCurve.GetPointHomogeneous(i)*(1-Alpha);
	}
	CtrlPoints[Degree] = InCurve.GetPointHomogeneous(FromDegree);
}

// The de Casteljau Algorithm (with weight)
template<int32 Dim, int32 Degree>
inline TVectorX<Dim+1> TRationalBezierCurve<Dim, Degree>::Split(TRationalBezierCurve<Dim, Degree>& OutFirst, TRationalBezierCurve<Dim, Degree>& OutSecond, double T) const
{
	double U = 1.0 - T;
	constexpr int32 DoubleDegree = Degree << 1;

	TVectorX<Dim+1> SplitCtrlPoints[DoubleDegree + 1];
	//TVecLib<Dim+1>::CopyArray(SplitCtrlPoints, CtrlPoints, Degree + 1);
	for (int32 i = 0; i <= Degree; ++i) {
		SplitCtrlPoints[i << 1] = CtrlPoints[i];
	}

	for (int32 j = 1; j <= Degree; ++j) {
		for (int32 i = 0; i <= Degree - j; ++i) {
			int32 i2 = i << 1;
			// P(j) and P(DoubleDegree - j) is determined
			SplitCtrlPoints[j + i2] = SplitCtrlPoints[j + i2 - 1] * U + SplitCtrlPoints[j + i2 + 1] * T;
		}
	}
	// Split(i,j): P(0,0), P(0,1), ..., P(0,n); P(0,n), P(1,n-1), ..., P(n,0).
	OutFirst.Reset(SplitCtrlPoints);
	OutSecond.Reset(SplitCtrlPoints + Degree);
	return SplitCtrlPoints[Degree];
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::Horner(double T) const
{
	double U = 1.0 - T;
	double Combination = 1;
	double TN = 1;
	TVectorX<Dim+1> Tmp = CtrlPoints[0] * U;
	for (int32 i = 1; i < Degree; ++i) {
		TN = TN * T;
		Combination *= static_cast<double>(Degree - i + 1) / i;
		Tmp = (Tmp + TN*Combination*CtrlPoints[i]) * U;
	}
	return TVector<Dim+1>::Projection(Tmp + TN*T*CtrlPoints[Degree]);
}

// The de Casteljau Algorithm 
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::DeCasteljau(double T) const
{
	double U = 1.0 - T;
	constexpr int32 DoubleDegree = Degree << 1;
	TVectorX<Dim+1> CalCtrlPoints[Degree + 1];
	TVecLib<Dim+1>::CopyArray(CalCtrlPoints, CtrlPoints, Degree + 1);
	//SplitCtrlPoints[0] = CtrlPoints[0];
	//SplitCtrlPoints[DoubleDegree] = CtrlPoints[Degree];
	for (int32 j = 1; j <= Degree; ++j) {
		for (int32 i = 0; i <= Degree - j; ++i) {
			CalCtrlPoints[i] = CalCtrlPoints[i] * U + CalCtrlPoints[i + 1] * T;
		}
	}
	return TVecLib<Dim+1>::Projection(CalCtrlPoints[0]);
}
