// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "BezierCurve.h"

#pragma once

// The de Casteljau Algorithm (or Horner's Algorithm if necessary)
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TBezierCurve<Dim, Degree>::GetPosition(double T) const
{
	//if (Dim >= 5) {
	//	return GetPositionDirectly(T);
	//}
	return GetPositionIteratively(T);
}

// Tangent not normalized. D(i) = (n / (t1 - t0)) * (P(i+1) - P(i))
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TBezierCurve<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 1)) {
		return TVecLib<Dim+1>::Projection(CtrlPoints[1] - CtrlPoints[0]);
	}
	TBezierCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);
	return Hodograph.GetPosition(T);
}

template<int32 Dim, int32 Degree>
inline double TBezierCurve<Dim, Degree>::GetPrincipalCurvature(double T, int32 Principal) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TBezierCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TBezierCurve<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::PrincipalCurvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T), Principal);
}

template<int32 Dim, int32 Degree>
inline double TBezierCurve<Dim, Degree>::GetCurvature(double T) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TBezierCurve<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TBezierCurve<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::Curvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T));
}

// Using Taylor's Series: B(t) = Sum{ 1/n! * (d^n(B)/dt^n)(t) * t^n }
template<int32 Dim, int32 Degree>
inline void TBezierCurve<Dim, Degree>::ToPolynomialForm(TVectorX<Dim+1>* OutPolyForm) const
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

template<int32 Dim, int32 Degree>
inline void TBezierCurve<Dim, Degree>::CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const
{
	for (int32 i = 0; i < Degree; ++i) {
		OutHodograph.SetPoint(i, TVectorX<Dim>(CtrlPoints[i + 1] - CtrlPoints[i]), 1.);
	}
}

template<int32 Dim, int32 Degree>
inline void TBezierCurve<Dim, Degree>::ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& InCurve)
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

// The de Casteljau Algorithm 
template<int32 Dim, int32 Degree>
inline void TBezierCurve<Dim, Degree>::Split(TBezierCurve<Dim, Degree>& OutFirst, TBezierCurve<Dim, Degree>& OutSecond, double T)
{
	double U = 1.0 - T;
	constexpr int32 DoubleDegree = Degree << 1;
	//constexpr int32 HalfDegree = Degree >> 1;
	TVectorX<Dim+1> SplitCtrlPoints[DoubleDegree + 1];
	TVectorX<Dim+1>::CopyArray(SplitCtrlPoints, CtrlPoints, Degree + 1);
	//SplitCtrlPoints[0] = CtrlPoints[0];
	//SplitCtrlPoints[DoubleDegree] = CtrlPoints[Degree];
	for (int32 j = 1; j <= Degree; ++j) {
		// P(2n-j) = P(n-j,j)
		SplitCtrlPoints[DoubleDegree - j + 1] = SplitCtrlPoints[Degree - j + 1];
		for (int32 i = 0; i <= Degree - j; ++i) {
			// P(j) is determined
			SplitCtrlPoints[j + i] = SplitCtrlPoints[j + i - 1] * U + SplitCtrlPoints[j + i] * T;
		}
	}
	// Split(i,j): P(0,0), P(0,1), ..., P(0,n); P(0,n), P(1,n-1), ..., P(n,0).
	OutFirst.Reset(SplitCtrlPoints);
	OutSecond.Reset(SplitCtrlPoints + Degree);
}

// Horner's Algorithm
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TBezierCurve<Dim, Degree>::GetPositionDirectly(double T) const
{
	double U = 1.0 - T;
	double Combination = 1;
	double TN = 1;
	TVectorX<Dim> Tmp = TVectorX<Dim+1>::Projoection(CtrlPoints[0]) * U;
	for (int32 i = 1; i < Degree; ++i) {
		TN = TN * T;
		Combination *= static_cast<double>(Degree - i + 1) / i;
		Tmp = (Tmp + TN*Combination*TVector<Dim+1>::Projection(CtrlPoints[i])) * U;
	}
	return Tmp + TN*T*TVector<Dim+1>::Projection(CtrlPoints[Degree]);
}

// The de Casteljau Algorithm 
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TBezierCurve<Dim, Degree>::GetPositionIteratively(double T) const
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
