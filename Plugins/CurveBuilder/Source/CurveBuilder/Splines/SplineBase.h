// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Utils/LinearAlgebraUtils.h"

template<int32 Dim, int32 Degree = 3>
class TSplineBase
{
public:
	FORCEINLINE TSplineBase(){}

	virtual ~TSplineBase() {}

public:
	FORCEINLINE static constexpr int32 SplineDim() { return Dim; }
	FORCEINLINE static constexpr int32 SplineDimHomogeneous() { return Dim + 1; }
	FORCEINLINE static constexpr int32 SplineDegree() { return Degree; }
	FORCEINLINE static constexpr int32 SplineRank() { return Degree + 1; }

	FORCEINLINE TVectorX<Dim> GetNormalizedTangent(double T) const { return GetTangent(T).GetSafeNormal(); }

	double GetLength(double T) const
	{
		TTuple<double, double> ParamRange = GetParamRange();
		// if (Degree < 5) 
		TGaussLegendre<GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, ParamRange.Get<0>(), ParamRange.Get<1>());
		return GaussLegendre.Integrate(T);
	}

	double GetParameterAtLength(double S) const
	{
		TTuple<double, double> ParamRange = GetParamRange();
		// if (Degree < 5) 
		TGaussLegendre<GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, ParamRange.Get<0>(), ParamRange.Get<1>());
		return GaussLegendre.SolveFromIntegration(S);
	}

	FORCEINLINE void AddPointAtLast(const TVectorX<Dim+1>& Point, double Param)
	{
		//AddPointAtLast(TVectorX<Dim>(Point), TVecLib<Dim+1>::Last(Point));
		AddPointAtLast(TVecLib<Dim+1>::Projection(Point), Param, TVecLib<Dim+1>::Last(Point));
	}

	FORCEINLINE void AddPointAt(const TVectorX<Dim+1>& Point, TOptional<double> Param = TOptional<double>(), int32 Index = 0)
	{
		//AddPointAt(TVectorX<Dim>(Point), Index, TVecLib<Dim+1>::Last(Point));
		AddPointAt(TVecLib<Dim+1>::Projection(Point), Param, Index, TVecLib<Dim+1>::Last(Point));
	}
public:

	virtual void AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) = 0;

	virtual void AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) = 0;

	virtual void AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), int32 Index = 0, double Weight = 1.) = 0;

	virtual void RemovePointAt(int32 Index = 0) = 0;

	// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	virtual void RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom = 0) = 0;

	// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	virtual void AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 NthPointOfFrom = 0) = 0;

	//// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	//virtual void AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom = 0) = 0;

	virtual void Reverse() = 0;

	virtual TVectorX<Dim> GetPosition(double T) const = 0;

	virtual TVectorX<Dim> GetTangent(double T) const = 0;

	virtual double GetPrincipalCurvature(double T, int32 Principal = 0) const = 0;

	virtual double GetCurvature(double T) const = 0;

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<Dim+1> > >& OutPolyForms) const = 0;

	virtual TTuple<double, double> GetParamRange() const = 0;

	virtual bool FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr = 1.) const = 0;

public:
	static double ConvertRange(double T, const TTuple<double, double>& RangeFrom, const TTuple<double, double>& RangeTo)
	{
		double DiffFrom = RangeFrom.Get<1>() - RangeFrom.Get<0>();
		if (FMath::IsNearlyZero(DiffFrom)) {
			return 0.;
		}
		double TN = (T - RangeFrom.Get<0>()) / DiffFrom;
		return RangeTo.Get<0>() * (1 - TN) + RangeTo.Get<1>() * TN;
	}
};
