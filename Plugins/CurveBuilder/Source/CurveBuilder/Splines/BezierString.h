// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineBase.h"
#include "Curves/BezierCurve.h"

template<int32 Dim>
struct TBezierStringControlPoint
{
	TBezierStringControlPoint() : Pos(TVecLib<Dim+1>::Zero()), Param(0.) {}
	TBezierStringControlPoint(const TVectorX<Dim+1>& InPos, double InParam = 0.) : Pos(InPos), Param(InParam) {}

	TVectorX<Dim+1> Pos;
	double Param;
};

// Clamped B-Spline
template<int32 Dim, int32 Degree = 3>
class TBezierString : TSplineBase<Dim, Degree>
{
	using TSplineBase<Dim, Degree>::TSplineBase;
public:
	using FPointNode = typename TDoubleLinkedList<TBezierStringControlPoint<Dim> >::TDoubleLinkedListNode;
public:
	FORCEINLINE TBezierString() {}

	FORCEINLINE TBezierString(const TBezierString<Dim, Degree>& InSpline);

	FORCEINLINE void Reset() { CtrlPointsList.Empty(); }

	virtual ~TClampedBSpline() { CtrlPointsList.Empty(); }

	FORCEINLINE int32 GetCtrlPointNum() const
	{
		return CtrlPointsList.Num();
	}

public:
	FPointNode* FindNodeByParam(double Param, int32 NthNode = 0) const;

	FPointNode* FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode = 0) const;

	void GetOpenFormPointsAndParams(TArray<TVectorX<Dim+1> >& CtrlPoints, TArray<double>& Params) const;

public:
	virtual void CreateHodograph(TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const;

	virtual void Split(TClampedBSpline<Dim, Degree>& OutFirst, TClampedBSpline<Dim, Degree>& OutSecond, double T);

	virtual void AddPointAtLast(const TBezierStringControlPoint<Dim>& PointStruct);

	virtual void AddPointAt(const TBezierStringControlPoint<Dim>& PointStruct, int32 Index = 0);

	virtual void AddPointWithParamWithoutChangingShape(double Param);

public:
	virtual void AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) override;

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
	TDoubleLinkedList<TBezierStringControlPoint<Dim> > CtrlPointsList;

	// Reference: https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
	TVectorX<Dim> DeBoor(double T, const TArray<TVectorX<Dim+1> >& CtrlPoints, const TArray<double>& Params) const;

	// Reference: https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
	TVectorX<Dim> CoxDeBoor(double T, const TArray<TVectorX<Dim+1> >& CtrlPoints, const TArray<double>& Params) const;
};

#include "BezierString.inl"
