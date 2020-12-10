// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"

template<int32 Dim, int32 Degree>
class TOffsetBase
{
public:
	TOffsetBase(double InFromP = 0., double InToP = 1.) : FromP(InFromP), ToP(InToP) {}

	FORCEINLINE static constexpr int32 SplineDim() { return Dim; }
	FORCEINLINE static constexpr int32 SplineDimHomogeneous() { return Dim + 1; }
	FORCEINLINE static constexpr int32 SplineDegree() { return Degree; }
	FORCEINLINE static constexpr int32 SplineOrder() { return Degree + 1; }

	FORCEINLINE TVectorX<Dim> GetNormalizedTangent(double T) const { return GetTangent(T).GetSafeNormal(); }

public:

	virtual void Reverse() {}

	virtual double GetPlanCurvature(double T, int32 PlanIndex = 0) const { return -1; }

	virtual double GetCurvature(double T) const { return -1; }

	virtual TTuple<double, double> GetParamRange() const { return MakeTuple(-1., -1.); }

	template<int32 DimOri>
	virtual void MakeCurves(TArray<TBezierCurve<DimOri, Degree> >& OutCurves, const TSplineBase<DimOri, Degree>& InOriginalSpline) const = 0;

protected:
	virtual TVectorX<Dim> GetPosition(double T) const { return TVecLib<Dim>::Zero(); }

	virtual TVectorX<Dim> GetTangent(double T) const { return TVecLib<Dim>::Zero(); }

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<Dim+1> > >& OutPolyForms) const {}


protected:
	double FromP, ToP;

};
