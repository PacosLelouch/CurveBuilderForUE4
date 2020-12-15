// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "OffsetBase.h"
#include "../Splines/SplineBase.h"

enum class EOffsetDirectionType : uint8
{
	DirT,
	DirZ,
};

struct FOffsetType
{
	EOffsetDirectionType DirectionType = EOffsetDirectionType::DirZ;
	double Sgn = 1.;
};


template<int32 Degree>
class TOffsetExplicit2Base : public TOffsetBase<2, Degree>
{
public:
	using TOffsetBase<2, Degree>::TOffsetBase;

	FORCEINLINE ESplineType GetType() const { return Type; }

	double GetLength(double T) const
	{
		TTuple<double, double> ParamRange = GetParamRange();
		// if (Degree < 5) 
		TGaussLegendre<NumericalCalculationConst::GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, ParamRange.Get<0>(), ParamRange.Get<1>());
		return GaussLegendre.Integrate(T);
	}

	double GetParameterAtLength(double S) const
	{
		TTuple<double, double> ParamRange = GetParamRange();
		// if (Degree < 5) 
		TGaussLegendre<NumericalCalculationConst::GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, ParamRange.Get<0>(), ParamRange.Get<1>());
		return GaussLegendre.SolveFromIntegration(S);
	}

	virtual TVectorX<2> GetValue(double T) const 
	{ 
		return GetPosition(T)[1]; 
	}

	virtual bool FindParamByValue(double& OutParam, double Value, double ToleranceSqr = 1.) const
	{
		return false;
	}

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<3> > >& OutPolyForms) const override {}

	virtual int32 GetCtrlPointNum() const
	{
		return -1;
	}

	virtual TSharedRef<TOffsetExplicit2Base<Degree> > CreateSameType(int32 EndContinuity = -1) const
	{
		return MakeShared<TOffsetExplicit2Base<Degree> >();
	}

	virtual TSharedRef<TOffsetExplicit2Base<Degree> > Copy() const
	{
		return MakeShared<TOffsetExplicit2Base<Degree> >();
	}

	virtual void ProcessBeforeCreateSameType() {}

	virtual void AddPointAtLast(double Value, TOptional<double> Param = TOptional<double>(), double Weight = 1.) {}

	virtual void AddPointAtFirst(double Value, TOptional<double> Param = TOptional<double>(), double Weight = 1.) {}

	virtual void AddPointAt(double Value, TOptional<double> Param = TOptional<double>(), int32 Index = 0, double Weight = 1.) {}

	virtual void InsertPoint(double Value, double Param, double Weight = 1.) {}

	virtual void RemovePointAt(int32 Index = 0) {}

	// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	virtual void RemovePoint(double Param, int32 NthPointOfFrom = 0) {}

	// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	virtual bool AdjustCtrlPointValue(double FromP, double ToV, int32 TangentFlag = 0, int32 NthPointOfFrom = 0, double ToleranceSqr = 1.) { return false; }

	//// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	//virtual void AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom = 0) {}

	virtual double GetPlanCurvature(double T, int32 PlanIndex = 0) const override { return -1; }

	virtual double GetCurvature(double T) const override { return -1; }

	virtual TTuple<double, double> GetParamRange() const override { return MakeTuple(-1., -1.); }

protected:
	virtual TVectorX<2> GetPosition(double T) const override { return TVecLib<2>::Zero(); }

	virtual TVectorX<2> GetTangent(double T) const override { return TVecLib<2>::Zero(); }

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<3> > >& OutPolyForms) const override {}

public:

	virtual void GetKnotsS(TArray<double>& OutKnotsS) const {}

	template<int32 DimOri>
	virtual void MakeCurves(TArray<TBezierCurve<DimOri, Degree> >& OutCurves, const TSplineBase<DimOri, Degree>& InOriginalSpline) const override;

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

protected:
	ESplineType SplineType = ESplineType::Unknown;
	FOffsetType OffsetType;
};

#include "OffsetExplicitBase.inl"
