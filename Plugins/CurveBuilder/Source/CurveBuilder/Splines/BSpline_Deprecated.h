// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineBase.h"
#include "Containers/List.h"
#include "Utils/LinearAlgebraUtils.h"
#include "Utils/NumericalCalculationUtils.h"
#include "Curves/BezierCurve.h"

template<int32 Dim>
struct TClampedBSplineControlPoint
{
	TClampedBSplineControlPoint() : Pos(TVecLib<Dim+1>::Zero()), Param(0.) {}
	TClampedBSplineControlPoint(const TVectorX<Dim+1>& InPos, double InParam = 0.) : Pos(InPos), Param(InParam) {}
	TClampedBSplineControlPoint(const TClampedBSplineControlPoint<Dim>& InP) : Pos(InP.Pos), Param(InP.Param) {}
	TClampedBSplineControlPoint<Dim>& operator=(const TClampedBSplineControlPoint<Dim>& InP) { Pos = InP.Pos; Param = InP.Param; return *this; }

	TVectorX<Dim+1> Pos;
	double Param = 0.;
	int32 MiddleRepeatNum = 0;
};

// Clamped B-Spline
template<int32 Dim, int32 Degree = 3>
class TClampedBSpline : TSplineBase<Dim, Degree>
{
	using TSplineBase<Dim, Degree>::TSplineBase;
public:
	using FPointNode = typename TDoubleLinkedList<TClampedBSplineControlPoint<Dim> >::TDoubleLinkedListNode;
public:
	FORCEINLINE TClampedBSpline() {}

	FORCEINLINE TClampedBSpline(const TClampedBSpline<Dim, Degree>& InSpline);

	FORCEINLINE TClampedBSpline<Dim, Degree>& operator=(const TClampedBSpline<Dim, Degree>& InSpline);

	FORCEINLINE void Reset() { CtrlPointsList.Empty(); }

	virtual ~TClampedBSpline() { CtrlPointsList.Empty(); }

	FORCEINLINE int32 GetCtrlPointNum() const
	{
		return CtrlPointsList.Num();
	}

	FORCEINLINE FPointNode* FirstNode() const
	{
		return CtrlPointsList.GetHead();
	}

	FORCEINLINE FPointNode* LastNode() const
	{
		return CtrlPointsList.GetTail();
	}

public:
	FPointNode* FindNodeByParam(double Param, int32 NthNode = 0) const;

	FPointNode* FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode = 0) const;

	void GetOpenFormPointsAndParams(TArray<TVectorX<Dim+1> >& CtrlPoints, TArray<double>& Params) const;

	void GetCtrlPointsAndParams(TArray<TVectorX<Dim+1> >& CtrlPoints, TArray<double>& Params) const;

public:
	virtual void CreateHodograph(TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const;

	virtual TVectorX<Dim+1> Split(
		TClampedBSpline<Dim, Degree>& OutFirst, TClampedBSpline<Dim, Degree>& OutSecond, double T,
		TArray<TArray<TVectorX<Dim+1> > >* SplitPosArray = nullptr, TArray<TArray<double> >* SplitParamArray = nullptr) const;

	virtual void AddPointAtLast(const TClampedBSplineControlPoint<Dim>& PointStruct);

	virtual void AddPointAtFirst(const TClampedBSplineControlPoint<Dim>& PointStruct);

	virtual void AddPointAt(const TClampedBSplineControlPoint<Dim>& PointStruct, int32 Index = 0);

	virtual void AddPointWithParamWithoutChangingShape(double T);

	virtual void ToBezierString(TArray<TBezierCurve<Dim, Degree> >& Beziers) const;

public:
	virtual void AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) override;

	virtual void AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) override;

	virtual void AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), int32 Index = 0, double Weight = 1.) override;

	virtual void RemovePointAt(int32 Index = 0) override;

	virtual void RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom = 0) override;

	virtual void RemovePoint(double Param, int32 NthPointOfFrom = 0) override;

	virtual void AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 NthPointOfFrom = 0) override;

	virtual void AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom = 0) override;

	virtual void Reverse() override;

	virtual TVectorX<Dim> GetPosition(double T) const override;

	virtual TVectorX<Dim> GetTangent(double T) const override;

	virtual double GetPrincipalCurvature(double T, int32 Principal = 0) const override;

	virtual double GetCurvature(double T) const override;

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<Dim+1> > >& OutPolyForms) const override;

	virtual TTuple<double, double> GetParamRange() const override;

protected:
	TDoubleLinkedList<TClampedBSplineControlPoint<Dim> > CtrlPointsList;

	// DeBoor is more efficient than Cox-DeBoor. Reference: https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
	TVectorX<Dim+1> DeBoor(double T, const TArray<TVectorX<Dim+1> >& CtrlPoints, const TArray<double>& Params,
		TArray<TArray<TVectorX<Dim+1> > >* SplitPosArray = nullptr, TArray<TArray<double> >* SplitParamArray = nullptr) const;

	// Reference: https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
	TVectorX<Dim+1> CoxDeBoor(double T, const TArray<TVectorX<Dim+1> >& CtrlPoints, const TArray<double>& Params) const;
};

#include "BSpline.inl"