#include "BezierCurve.h"
#include "RationalBezierCurve.h"
// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

// Horner's Algorithm
template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetPosition(double T) const
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

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TRationalBezierCurve<Dim, Degree>::GetTangent(double T) const
{
	return TVectorX<Dim>();//TODO
}

template<int32 Dim, int32 Degree>
inline double TRationalBezierCurve<Dim, Degree>::GetPrincipalCurvature(double T, int32 Principal) const
{
	return 0.0;//TODO
}

template<int32 Dim, int32 Degree>
inline double TRationalBezierCurve<Dim, Degree>::GetCurvature(double T) const
{
	return 0.0;//TODO
}

// Using Taylor's Series: B(t) = Sum{ 1/n! * (d^n(B)/dt^n)(t) * t^n }
template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::ToPolynomialForm(TVectorX<Dim+1> OutPolyForm[Degree + 1]) const
{
	TVectorX<Dim+1> DTable[Degree + 1];
	TVectorX<Dim+1>::CopyArray(DTable, CtrlPoints, Degree + 1);
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
inline void TRationalBezierCurve<Dim, Degree>::CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree)>& OutHodograph) const
{
	//TODO
}

// The de Casteljau Algorithm 
template<int32 Dim, int32 Degree>
inline void TRationalBezierCurve<Dim, Degree>::Split(TRationalBezierCurve<Dim, Degree>& OutFirst, TRationalBezierCurve<Dim, Degree>& OutSecond, double T)
{
	double U = 1.0 - T;
	//TODO
}
