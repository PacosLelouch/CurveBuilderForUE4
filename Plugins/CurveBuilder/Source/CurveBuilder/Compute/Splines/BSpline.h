// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineBase.h"
#include "Containers/List.h"
#include "Utils/LinearAlgebraUtils.h"
#include "Utils/NumericalCalculationUtils.h"
#include "Curves/BezierCurve.h"

template<int32 Dim, int32 Degree = 3>
struct TClampedBSplineControlPoint : public TSplineBaseControlPoint<Dim, Degree>
{
	using Super = typename TSplineBaseControlPoint<Dim, Degree>;

	TClampedBSplineControlPoint() : Super(TVecLib<Dim+1>::Zero()) {}
	TClampedBSplineControlPoint(const TVectorX<Dim+1>& InPos) : Super(InPos) {}
	TClampedBSplineControlPoint(const TClampedBSplineControlPoint<Dim>& InP) : Super(InP.Pos) {}
	TClampedBSplineControlPoint<Dim>& operator=(const TClampedBSplineControlPoint<Dim>& InP) { Pos = InP.Pos; return *this; }

	virtual TSharedRef<TSplineBaseControlPoint<Dim, Degree> > Copy() const
	{
		return MakeShared<TClampedBSplineControlPoint<Dim, Degree> >(*this);
	}
};

// Clamped B-Spline
template<int32 Dim, int32 Degree = 3>
class TClampedBSpline : public TSplineBase<Dim, Degree>
{
	using TSplineBase<Dim, Degree>::TSplineBase;
public:
	using FControlPointType = typename TClampedBSplineControlPoint<Dim, Degree>;
	using FControlPointTypeRef = typename TSharedRef<FControlPointType>;
	using FPointNode = typename TDoubleLinkedList<FControlPointTypeRef>::TDoubleLinkedListNode;
public:
	FORCEINLINE TClampedBSpline() 
	{
		Type = ESplineType::ClampedBSpline;
	}

	FORCEINLINE TClampedBSpline(const TClampedBSpline<Dim, Degree>& InSpline);

	FORCEINLINE TClampedBSpline<Dim, Degree>& operator=(const TClampedBSpline<Dim, Degree>& InSpline);

	FORCEINLINE void Reset() { Type = ESplineType::ClampedBSpline; CtrlPointsList.Empty(); KnotIntervals.Empty(KnotIntervals.Num()); }

	FORCEINLINE void Reset(const TArray<TVectorX<Dim+1> >& InCtrlPoints, const TArray<double>& InKnotIntervals);

	virtual ~TClampedBSpline() { CtrlPointsList.Empty(); KnotIntervals.Empty(KnotIntervals.Num()); }

	FORCEINLINE int32 GetKnotNum() const
	{
		return KnotIntervals.Num();
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
	virtual int32 GetCtrlPointNum() const override
	{
		return CtrlPointsList.Num();
	}

	virtual void GetCtrlPointStructs(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>>& OutControlPointStructs) const override;

	virtual TWeakPtr<TSplineBaseControlPoint<Dim, Degree>> GetLastCtrlPointStruct() const override;

	virtual TWeakPtr<TSplineBaseControlPoint<Dim, Degree>> GetFirstCtrlPointStruct() const override;

	virtual void GetSegParams(TArray<double>& OutParameters) const override;

	virtual TSharedRef<TSplineBase<Dim, Degree> > CreateSameType(int32 EndContinuity = -1) const override;

	virtual TSharedRef<TSplineBase<Dim, Degree> > Copy() const override;

	virtual void ProcessBeforeCreateSameType(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>>* NewControlPointStructsPtr = nullptr) override;

	//FPointNode* FindNodeByParam(double Param, int32 NthNode = 0) const;

	FPointNode* FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode = 0, double ToleranceSqr = 1.) const;

	//void GetOpenFormPointsAndParams(TArray<TVectorX<Dim+1> >& CtrlPoints, TArray<double>& Params) const;

	//void GetCtrlPointsAndParams(TArray<TVectorX<Dim+1> >& CtrlPoints, TArray<double>& Params) const;

	void GetCtrlPoints(TArray<TVectorX<Dim+1> >& CtrlPoints) const;

	virtual bool ToBezierCurves(TArray<TBezierCurve<Dim, Degree> >& BezierCurves, TArray<TTuple<double, double> >* ParamRangesPtr = nullptr) const override;

	int32 CreateFromBezierCurves(const TArray<TBezierCurve<Dim, Degree>>& BezierCurves, double TOL = 1e-2);

	void GetClampedKnotIntervals(TArray<double>& OutClampedKnotIntervals) const;

	void GetKnotIntervals(TArray<double>& OutKnotIntervals) const;

	int32 GetMaxKnotIntervalIndex() const;

	void AddNewKnotIntervalIfNecessary(TOptional<double> Param = TOptional<double>());

	void RemoveKnotIntervalIfNecessary();

public:
	virtual void CreateHodograph(TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const;

	virtual TVectorX<Dim+1> Split(
		TClampedBSpline<Dim, Degree>& OutFirst, TClampedBSpline<Dim, Degree>& OutSecond, double T,
		TArray<TArray<TVectorX<Dim+1> > >* SplitPosArray = nullptr, int32* OutEndIntervalIndex = nullptr) const;

	virtual void AddPointAtLast(const TClampedBSplineControlPoint<Dim>& PointStruct);

	virtual void AddPointAtFirst(const TClampedBSplineControlPoint<Dim>& PointStruct);

	virtual void AddPointAt(const TClampedBSplineControlPoint<Dim>& PointStruct, int32 Index = 0);

	// Insert a knot.
	virtual FPointNode* AddPointWithParamWithoutChangingShape(double T);

	virtual bool AdjustCtrlPointPos(FPointNode* Node, const TVectorX<Dim>& To, int32 NthPointOfFrom = 0);

	//virtual void AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom = 0);

	//virtual void RemovePoint(double Param, int32 NthPointOfFrom = 0);

public:
	virtual void AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) override;

	virtual void AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) override;

	virtual void AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), int32 Index = 0, double Weight = 1.) override;

	virtual void RemovePointAt(int32 Index = 0) override;

	virtual void RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom = 0) override;

	virtual void RemovePoint(const TSplineBaseControlPoint<Dim, Degree>& TargetPointStruct) override;

	virtual bool AdjustCtrlPointPos(TSplineBaseControlPoint<Dim, Degree>& PointStructToAdjust, const TVectorX<Dim>& To, int32 TangentFlag = 0, int32 NthPointOfFrom = 0) override;

	virtual bool AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 TangentFlag = 0, int32 NthPointOfFrom = 0, double ToleranceSqr = 1.) override;

	virtual void Reverse() override;

	virtual TVectorX<Dim> GetPosition(double T) const override;

	virtual TVectorX<Dim> GetTangent(double T) const override;

	virtual double GetPlanCurvature(double T, int32 PlanIndex = 0) const override;

	virtual double GetCurvature(double T) const override;

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<Dim+1> > >& OutPolyForms) const override;

	virtual TTuple<double, double> GetParamRange() const override;

	virtual bool FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr = 1.) const override;

	virtual bool FindParamsByComponentValue(TArray<double>& OutParams, double InValue, int32 InComponentIndex = 0, double ToleranceSqr = 1.) const override;

protected:
	TDoubleLinkedList<FControlPointTypeRef> CtrlPointsList;
	TArray<double> KnotIntervals;

	// DeBoor is more efficient than Cox-DeBoor. Reference: https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
	TVectorX<Dim+1> DeBoor(double T, const TArray<TVectorX<Dim+1> >& CtrlPoints, const TArray<double>& Params,
		TArray<TArray<TVectorX<Dim+1> > >* OutSplitPosArray = nullptr, int32* OutEndIntervalIndex = nullptr) const;

	// Reference: https://en.wikipedia.org/wiki/De_Boor%27s_algorithm
	TVectorX<Dim+1> CoxDeBoor(double T, const TArray<TVectorX<Dim+1> >& CtrlPoints, const TArray<double>& Params) const;

	void AddPointAtTailRaw(const TVectorX<Dim+1>& CtrlPoint);

	void AddKnotAtTailRaw(double Param);

	template<int32 SubDegree = Degree>
	int32 DetermineContinuity(TOptional<double>& OutParamRatio, const TBezierCurve<Dim, SubDegree>& Bezier1, const TBezierCurve<Dim, SubDegree>& Bezier2, double TOL);
};

template<int32 Dim, int32 Degree>
struct TSplineTraitByType<ESplineType::ClampedBSpline, Dim, Degree>
{
	using FSplineType = typename TClampedBSpline<Dim, Degree>;
	using FControlPointType = typename TClampedBSplineControlPoint<Dim, Degree>;
};

#include "BSpline.inl"
