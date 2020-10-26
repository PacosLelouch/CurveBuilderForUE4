#include "BezierCurve.h"
// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

// Horner's Algorithm
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TBezierCurve<Dim, Degree>::GetPosition(double T) const
{
	double U = 1.0 - T;
	double Combination = 1; 
	double TN = 1; 
	TVectorX<Dim> Tmp = CtrlPoints[0].NonHomogeneous() * U;
	for (int32 i = 1; i < Degree; ++i) { 
		TN = TN * T; 
		Combination *= static_cast<double>(Degree - i + 1) / i; 
		Tmp = (Tmp + TN*Combination*CtrlPoints[i].NonHomogeneous()) * U;
	} 
	return (Tmp + TN*T*CtrlPoints[Degree].NonHomogeneous());
}

// Tangent not normalized. D(i) = (n / (t1 - t0)) * (P(i+1) - P(i))
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TBezierCurve<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 1)) {
		return TVectorX<Dim+1>(CtrlPoints[1] - CtrlPoints[0]).NonHomogeneous();
	}
	TBezierCurve<Dim, CLAMP_DEGREE(Degree)> Hodograph;
	CreateHodograph(Hodograph);
	return Hodograph.GetPosition(T);
}

template<int32 Dim, int32 Degree>
inline double TBezierCurve<Dim, Degree>::GetPrincipalCurvature(double T, int32 Principal) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TBezierCurve<Dim, CLAMP_DEGREE(Degree)> Hodograph;
	CreateHodograph(Hodograph);

	TBezierCurve<Dim, CLAMP_DEGREE(Degree-1)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVectorX<Dim>::PrincipalCurvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T), Principal);
}

template<int32 Dim, int32 Degree>
inline double TBezierCurve<Dim, Degree>::GetCurvature(double T) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TBezierCurve<Dim, CLAMP_DEGREE(Degree)> Hodograph;
	CreateHodograph(Hodograph);

	TBezierCurve<Dim, CLAMP_DEGREE(Degree-1)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVectorX<Dim>::Curvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T));
}

// Using Taylor's Series: B(t) = Sum{ 1/n! * (d^n(B)/dt^n)(t) * t^n }
template<int32 Dim, int32 Degree>
inline void TBezierCurve<Dim, Degree>::ToPolynomialForm(TVectorX<Dim+1>* OutPolyForm) const
{
	TVectorX<Dim+1> DTable[Degree + 1];
	TVectorX<Dim+1>::CopyArray(DTable, CtrlPoints, Degree + 1);
	//FMemory::Memcpy(DTable, CtrlPoints, (Degree + 1) * sizeof(TVectorX<Dim + 1>));
	double Combination = 1;
	OutPolyForm[0] = CtrlPoints[0];
	OutPolyForm[0].Last() = 1.;
	for (int32 i = 1; i <= Degree; ++i) {
		for (int32 j = 0; j <= Degree - i; ++j) {
			DTable[j] = DTable[j + 1] - DTable[j];
		}
		Combination *= static_cast<double>(Degree - i + 1) / i;
		OutPolyForm[i] = DTable[0] * Combination;
		OutPolyForm[0].Last() = 1.;
	}
}

template<int32 Dim, int32 Degree>
inline void TBezierCurve<Dim, Degree>::CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree)>& OutHodograph) const
{
	for (int32 i = 0; i < Degree; ++i) {
		OutHodograph.SetPoint(i, TVectorX<Dim>(CtrlPoints[i + 1] - CtrlPoints[i]));
	}
}

// The de Casteljau Algorithm 
template<int32 Dim, int32 Degree>
inline void TBezierCurve<Dim, Degree>::Split(TBezierCurve<Dim, Degree>& OutFirst, TBezierCurve<Dim, Degree>& OutSecond, double T)
{
	double U = 1.0 - T;
	constexpr int32 DoubleDegree = Degree << 1;
	constexpr int32 HalfDegree = Degree >> 1;
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
	OutFirst = TBezierCurve<Dim, Degree>(SplitCtrlPoints);
	OutSecond = TBezierCurve<Dim, Degree>(SplitCtrlPoints + Degree);
}
