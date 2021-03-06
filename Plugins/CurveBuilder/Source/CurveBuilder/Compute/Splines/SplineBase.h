// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Utils/LinearAlgebraUtils.h"
#include "../Curves/BezierCurve.h"

namespace SplineDataVersion
{
	static constexpr uint32 Major = 1;
	static constexpr uint32 Minor = 0;
}

enum class EContactType : uint8
{
	Start, End,
};

enum class ESplineType : uint8
{
	Unknown,
	ClampedBSpline,
	BezierString,
};

template<int32 Dim, int32 Degree = 3>
struct TSplineBaseControlPoint
{
	TSplineBaseControlPoint() : Pos(TVecLib<Dim+1>::Zero()) {}
	TSplineBaseControlPoint(const TVectorX<Dim+1>& InPos) : Pos(InPos) {}

	virtual TSharedRef<TSplineBaseControlPoint<Dim, Degree>> Copy() const
	{
		return MakeShared<TSplineBaseControlPoint<Dim, Degree>>(*this);
	}

	TVectorX<Dim+1> Pos;
};

template<int32 Dim, int32 Degree = 3>
class TSplineBase
{
public:
	using FControlPointType = typename TSplineBaseControlPoint<Dim, Degree>;
	using FControlPointTypeRef = typename TSharedRef<FControlPointType>;
	using FPointNode = typename TDoubleLinkedList<FControlPointTypeRef>::TDoubleLinkedListNode;
public:
	FORCEINLINE TSplineBase() {}

	virtual ~TSplineBase() {}

public:
	FORCEINLINE static constexpr int32 SplineDim() { return Dim; }
	FORCEINLINE static constexpr int32 SplineDimHomogeneous() { return Dim + 1; }
	FORCEINLINE static constexpr int32 SplineDegree() { return Degree; }
	FORCEINLINE static constexpr int32 SplineOrder() { return Degree + 1; }

	FORCEINLINE TVectorX<Dim> GetNormalizedTangent(double T) const { return GetTangent(T).GetSafeNormal(); }

	FORCEINLINE ESplineType GetType() const { return Type; }

	double GetLength(double T) const
	{
		TArray<TBezierCurve<Dim, Degree>> BezierCurves;
		TArray<TTuple<double, double>> ParamSegsPair;
		if (!ToBezierCurves(BezierCurves, &ParamSegsPair))
		{
			return 0.;
		}
		double Length = 0.;
		bool bShouldBreak = false;
		for (int32 i = 0; i < BezierCurves.Num() && !bShouldBreak; ++i)
		{
			double Start = ParamSegsPair[i].Get<0>(), End = ParamSegsPair[i].Get<1>(), Target = End;
			if (Start <= T && T <= End)
			{
				Target = T;
				bShouldBreak = true;
			}
			double De = End - Start;
			double NormalTarget = FMath::IsNearlyZero(De) ? 0.5 : (Target - Start) / De;
			Length += BezierCurves[i].GetLength(NormalTarget);
		}
		return Length;

		//TTuple<double, double> ParamRange = GetParamRange();
		//// if (Degree < 5) 
		//TGaussLegendre<NumericalCalculationConst::GaussLegendreN> GaussLegendre([this](double InT) -> double {
		//	return TVecLib<Dim>::Size(GetTangent(InT));
		//	}, ParamRange.Get<0>(), ParamRange.Get<1>());
		//return GaussLegendre.Integrate(T);
	}

	double GetParameterAtLength(double S) const
	{
		static constexpr int32 Iteration = 8;
		TTuple<double, double> ParamRange = GetParamRange();
		// if (Degree < 5) 
		TGaussLegendre<NumericalCalculationConst::GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return TVecLib<Dim>::Size(GetTangent(InT));
		}, ParamRange.Get<0>(), ParamRange.Get<1>());
		return GaussLegendre.SolveFromIntegration(S, Iteration);
	}

	FORCEINLINE void AddPointAtLast(const TVectorX<Dim+1>& Point, double Param)
	{
		//AddEndPoint(TVectorX<Dim>(Point), TVecLib<Dim+1>::Last(Point));
		AddPointAtLast(TVecLib<Dim+1>::Projection(Point), Param, TVecLib<Dim+1>::Last(Point));
	}

	FORCEINLINE void AddPointAt(const TVectorX<Dim+1>& Point, TOptional<double> Param = TOptional<double>(), int32 Index = 0)
	{
		//AddPointAt(TVectorX<Dim>(Point), Index, TVecLib<Dim+1>::Last(Point));
		AddPointAt(TVecLib<Dim+1>::Projection(Point), Param, Index, TVecLib<Dim+1>::Last(Point));
	}
public:
	virtual int32 GetCtrlPointNum() const
	{
		return -1;
	}

	virtual void GetCtrlPointStructs(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>>& OutControlPointStructs) const {}

	virtual TWeakPtr<TSplineBaseControlPoint<Dim, Degree>> GetLastCtrlPointStruct() const { return nullptr; }

	virtual TWeakPtr<TSplineBaseControlPoint<Dim, Degree>> GetFirstCtrlPointStruct() const { return nullptr; }

	virtual void GetSegParams(TArray<double>& OutParameters) const {}

	virtual bool ToBezierCurves(TArray<TBezierCurve<Dim, Degree> >& BezierCurves, TArray<TTuple<double, double> >* ParamRangesPtr = nullptr) const { return false; }

	virtual TSharedRef<TSplineBase<Dim, Degree> > CreateSameType(int32 EndContinuity = -1) const 
	{
		return MakeShared<TSplineBase<Dim, Degree> >();
	}

	virtual TSharedRef<TSplineBase<Dim, Degree> > Copy() const
	{
		return MakeShared<TSplineBase<Dim, Degree> >();
	}

	virtual void ProcessBeforeCreateSameType(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>>* NewControlPointStructsPtr = nullptr) {}

	virtual void AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) {}

	virtual void AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) {}

	virtual void AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), int32 Index = 0, double Weight = 1.) {}

	virtual void RemovePointAt(int32 Index = 0) {}

	// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	virtual void RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom = 0) {}

	virtual void RemovePoint(const TSplineBaseControlPoint<Dim, Degree>& TargetPointStruct) {}

	virtual bool AdjustCtrlPointPos(TSplineBaseControlPoint<Dim, Degree>& PointStructToAdjust, const TVectorX<Dim>& To, int32 TangentFlag = 0, int32 NthPointOfFrom = 0) { return false; }

	// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	virtual bool AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 TangentFlag = 0, int32 NthPointOfFrom = 0, double ToleranceSqr = 1.) { return false; }

	//// NthPointOfFrom means if there are multiple points with the same positions, which point to adjust.
	//virtual void AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom = 0) {}

	virtual void Reverse() {}

	virtual TVectorX<Dim> GetPosition(double T) const { return TVecLib<Dim>::Zero(); }

	virtual TVectorX<Dim> GetTangent(double T) const { return TVecLib<Dim>::Zero(); }

	virtual double GetPlanCurvature(double T, int32 PlanIndex = 0) const { return -1; }

	virtual double GetCurvature(double T) const { return -1; }

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<Dim+1> > >& OutPolyForms) const {}

	virtual TTuple<double, double> GetParamRange() const { return MakeTuple(-1., -1.); }

	virtual bool FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr = 1.) const { return false; }
	
	virtual bool FindParamsByComponentValue(TArray<double>& OutParams, double InValue, int32 InComponentIndex = 0, double ToleranceSqr = 1.) const { return false; }

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
	ESplineType Type = ESplineType::Unknown;
};

template<ESplineType Type, int32 Dim = 3, int32 Degree = 3>
struct TSplineTraitByType
{
	using FSplineType = typename TSplineBase<Dim, Degree>;
	using FControlPointType = typename TSplineBaseControlPoint<Dim, Degree>;
};

template<typename TSClass>
struct TSplineTraitByClass
{
	using FSplineType = typename TSClass;
};

