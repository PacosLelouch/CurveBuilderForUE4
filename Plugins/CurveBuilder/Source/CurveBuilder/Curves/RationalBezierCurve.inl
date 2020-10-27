// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RationalBezierCurve.h"

#pragma once

// The de Casteljau Algorithm (or Horner's Algorithm if necessary)
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetPosition(double T) const
{
	//if (Dim >= 5) {
	//	return GetPositionDirectly(T);
	//}
	return GetPositionIteratively(T);
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 1)) {
		return TVectorX<Dim+1>(CtrlPoints[1] - CtrlPoints[0]).NonHomogeneous();
	}
	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-1)> Hodograph;
	CreateHodograph(Hodograph);
	return Hodograph.GetPosition(T);
}

template<int32 Dim, int32 Degree>
inline double TRationalBezierCurve<Dim, Degree>::GetPrincipalCurvature(double T, int32 Principal) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-1)> Hodograph;
	CreateHodograph(Hodograph);

	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-2)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVectorX<Dim>::PrincipalCurvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T), Principal);
}

template<int32 Dim, int32 Degree>
inline double TRationalBezierCurve<Dim, Degree>::GetCurvature(double T) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-1)> Hodograph;
	CreateHodograph(Hodograph);

	TRationalBezierCurve<Dim, CLAMP_DEGREE(Degree-2)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVectorX<Dim>::Curvature(Hodograph.GetPosition(T), Hodograph2.GetPosition(T));
}

// Using Taylor's Series: B(t) = Sum{ 1/n! * (d^n(B)/dt^n)(t) * t^n }
template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::ToPolynomialForm(TVectorX<Dim+1> OutPolyForm[Degree + 1]) const
{
	TVectorX<Dim+1> DTable[Degree + 1];
	TVectorX<Dim+1>::CopyArray(DTable, CtrlPoints, Degree + 1);
	double Combination = 1;
	OutPolyForm[0] = CtrlPoints[0];
	OutPolyForm[0].WeightToOne();
	OutPolyForm[0].Last() = 1.;
	for (int32 i = 1; i <= Degree; ++i) {
		for (int32 j = 0; j <= Degree - i; ++j) {
			DTable[j] = DTable[j + 1] - DTable[j];
		}
		Combination *= static_cast<double>(Degree - i + 1) / i;
		OutPolyForm[i] = DTable[0] * Combination;
		OutPolyForm[i].WeightToOne();
		OutPolyForm[i].Last() = 1.;
	}
}

// How to deal with the situation where w0 == 0.0?
template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1)>& OutHodograph) const
{
	for (int32 i = 0; i < Degree; ++i) {
		OutHodograph.SetPoint(i, TVectorX<Dim>(CtrlPoints[i + 1] - CtrlPoints[i]), CtrlPoints[i + 1].Last() / CtrlPoints[i].Last());
	}
}

template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1)>& InCurve) const
{
	constexpr int32 FromDegree = CLAMP_DEGREE(Degree-1);
	CtrlPoints[0] = InCurve.GetPointHomogeneous(0);
	for (int32 i = 1; i < Degree ++i) {
		double Alpha = static_cast<double>(i) / (Degree);
		CtrlPoints[i] = InCurve.GetPointHomogeneous(i - 1)*Alpha + InCurve.GetPointHomogeneous(i)*(1-Alpha);
	}
	CtrlPoints[Degree] = InCurve.GetPointHomogeneous(FromDegree);
}

// The de Casteljau Algorithm (with weight)
template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::Split(TRationalBezierCurve<Dim, Degree>& OutFirst, TRationalBezierCurve<Dim, Degree>& OutSecond, double T)
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
	OutFirst = TRationalBezierCurve<Dim, Degree>(SplitCtrlPoints);
	OutSecond = TRationalBezierCurve<Dim, Degree>(SplitCtrlPoints + Degree);
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetPositionDirectly(double T) const
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
	return (Tmp + TN*T*CtrlPoints[Degree]).NonHomogeneous();
}

// The de Casteljau Algorithm 
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetPositionIteratively(double T) const
{
	double U = 1.0 - T;
	constexpr int32 DoubleDegree = Degree << 1;
	TVectorX<Dim+1> CalCtrlPoints[Degree + 1];
	TVectorX<Dim+1>::CopyArray(CalCtrlPoints, CtrlPoints, Degree + 1);
	//SplitCtrlPoints[0] = CtrlPoints[0];
	//SplitCtrlPoints[DoubleDegree] = CtrlPoints[Degree];
	for (int32 j = 1; j <= Degree; ++j) {
		for (int32 i = 0; i <= Degree - j; ++i) {
			CalCtrlPoints[i] = CalCtrlPoints[i] * U + CalCtrlPoints[i + 1] * T;
		}
	}
	return CalCtrlPoints[0].NonHomogeneous();
}
