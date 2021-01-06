// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineBase.h"
#include "Containers/List.h"
#include "Utils/LinearAlgebraUtils.h"
#include "Utils/NumericalCalculationUtils.h"
#include "Curves/BezierCurve.h"
#include "CurveOperations/BezierOperations.h"
#include "Utils/Continuity.h"

template<int32 Dim>
struct TBezierString3ControlPoint : public TSplineBaseControlPoint<Dim, 3>
{
	using Super = typename TSplineBaseControlPoint<Dim, 3>;

	TBezierString3ControlPoint() : Super(TVecLib<Dim+1>::Zero()), PrevCtrlPointPos(TVecLib<Dim+1>::Zero()), NextCtrlPointPos(TVecLib<Dim+1>::Zero()), Param(0.) {}
	TBezierString3ControlPoint(const TVectorX<Dim+1>& InPos, double InParam = 0.) : Super(InPos), PrevCtrlPointPos(TVecLib<Dim+1>::Zero()), NextCtrlPointPos(TVecLib<Dim+1>::Zero()), Param(InParam) {}
	//TBezierString3ControlPoint(const TVectorX<Dim+1>& InPos, const TVectorX<Dim+1>& PrevPos, const TVectorX<Dim+1>& NextPos, double InParam = 0.) : Super(InPos), PrevCtrlPointPos(PrevPos), NextCtrlPointPos(NextPos), Param(InParam) {}
	TBezierString3ControlPoint(const TVectorX<Dim+1>& InPos, const TVectorX<Dim+1>& PrevPos, const TVectorX<Dim+1>& NextPos, double InParam = 0., EEndPointContinuity InCont = EEndPointContinuity::G1) : Super(InPos), PrevCtrlPointPos(PrevPos), NextCtrlPointPos(NextPos), Param(InParam), Continuity(InCont) {}
	TBezierString3ControlPoint(const TBezierString3ControlPoint<Dim>& InP) : Super(InP.Pos), PrevCtrlPointPos(InP.PrevCtrlPointPos), NextCtrlPointPos(InP.NextCtrlPointPos), Param(InP.Param) {}
	TBezierString3ControlPoint<Dim>& operator=(const TBezierString3ControlPoint<Dim>& InP) { Pos = InP.Pos; PrevCtrlPointPos = InP.PrevCtrlPointPos; NextCtrlPointPos = InP.NextCtrlPointPos; Param = InP.Param; return *this; }
	
	virtual TSharedRef<TSplineBaseControlPoint<Dim, 3> > Copy() const override
	{
		return MakeShared<TBezierString3ControlPoint<Dim> >(*this);
	}

	TVectorX<Dim+1> PrevCtrlPointPos, NextCtrlPointPos;
	double Param;
	EEndPointContinuity Continuity = EEndPointContinuity::G1;
};

// BezierString3
template<int32 Dim>
class TBezierString3 : public TSplineBase<Dim>
{
	using TSplineBase<Dim>::TSplineBase;
public:
	using FControlPointType = typename TBezierString3ControlPoint<Dim>;
	using FControlPointTypeRef = typename TSharedRef<FControlPointType>;
	using FPointNode = typename TDoubleLinkedList<FControlPointTypeRef>::TDoubleLinkedListNode;
public:
	FORCEINLINE TBezierString3() 
	{
		Type = ESplineType::BezierString;
	}

	FORCEINLINE TBezierString3(const TBezierString3<Dim>& InSpline);

	FORCEINLINE TBezierString3(const TArray<TBezierCurve<Dim, 3> >& InCurves);

	FORCEINLINE TBezierString3<Dim>& operator=(const TBezierString3<Dim>& InSpline);

	FORCEINLINE void FromCurveArray(const TArray<TBezierCurve<Dim, 3> >& InCurves);

	FORCEINLINE void Reset() { Type = ESplineType::BezierString; CtrlPointsList.Empty(); }

	FORCEINLINE void Reset(
		const TArray<TVectorX<Dim+1>>& InPos, 
		const TArray<TVectorX<Dim+1>>& InPrev, 
		const TArray<TVectorX<Dim+1>>& InNext, 
		const TArray<double>& InParams, 
		const TArray<EEndPointContinuity>& InContinuities);

	FORCEINLINE void RemakeC2() { UpdateBezierString(nullptr); }

	virtual ~TBezierString3() { CtrlPointsList.Empty(); }

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

	FPointNode* FindNodeGreaterThanParam(double Param, int32 NthNode = 0) const;

	FPointNode* FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode = 0, double ToleranceSqr = 1.) const;

	FPointNode* FindNodeByExtentPosition(const TVectorX<Dim>& ExtentPoint, bool bFront = true, int32 NthNode = 0, double ToleranceSqr = 1.) const;

	void GetCtrlPoints(TArray<TVectorX<Dim+1> >& CtrlPoints) const;

	void GetCtrlPointsPrev(TArray<TVectorX<Dim+1> >& CtrlPoints) const;

	void GetCtrlPointsNext(TArray<TVectorX<Dim+1> >& CtrlPoints) const;

	void GetCtrlParams(TArray<double>& CtrlParams) const;

	virtual bool ToBezierCurves(TArray<TBezierCurve<Dim, 3> >& BezierCurves, TArray<TTuple<double, double> >* ParamRangesPtr = nullptr) const override;

public:
	virtual int32 GetCtrlPointNum() const override
	{
		return CtrlPointsList.Num();
	}

	virtual void GetCtrlPointStructs(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, 3>>>& OutControlPointStructs) const override;

	virtual TWeakPtr<TSplineBaseControlPoint<Dim, 3>> GetLastCtrlPointStruct() const override;

	virtual TWeakPtr<TSplineBaseControlPoint<Dim, 3>> GetFirstCtrlPointStruct() const override;

	virtual void GetSegParams(TArray<double>& OutParameters) const override;

	virtual TSharedRef<TSplineBase<Dim, 3> > CreateSameType(int32 EndContinuity = -1) const override;

	virtual TSharedRef<TSplineBase<Dim, 3> > Copy() const override;

	virtual void ProcessBeforeCreateSameType(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, 3>>>* NewControlPointStructsPtr = nullptr) override;

	virtual void Split(TBezierString3<Dim>& OutFirst, TBezierString3<Dim>& OutSecond, double T) const;

	virtual void AddPointAtLast(const TBezierString3ControlPoint<Dim>& PointStruct);

	virtual void AddPointAtFirst(const TBezierString3ControlPoint<Dim>& PointStruct);

	virtual void AddPointAt(const TBezierString3ControlPoint<Dim>& PointStruct, int32 Index = 0);

	virtual FPointNode* AddPointWithParamWithoutChangingShape(double T);

	virtual void AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom = 0);

	virtual void ChangeCtrlPointContinuous(double From, EEndPointContinuity Continuity, int32 NthPointOfFrom = 0);

	virtual bool AdjustCtrlPointTangent(double From, const TVectorX<Dim>& To, bool bNext = true, int32 NthPointOfFrom = 0);

	virtual bool AdjustCtrlPointTangent(FPointNode* Node, const TVectorX<Dim>& To, bool bNext = true, int32 NthPointOfFrom = 0);

	virtual void RemovePoint(double Param, int32 NthPointOfFrom = 0);

	virtual bool AdjustCtrlPointPos(FPointNode* Node, const TVectorX<Dim>& To, int32 NthPointOfFrom = 0);

public:
	virtual void AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) override;

	virtual void AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), double Weight = 1.) override;

	virtual void AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param = TOptional<double>(), int32 Index = 0, double Weight = 1.) override;

	virtual void RemovePointAt(int32 Index = 0) override;

	virtual void RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom = 0) override;

	virtual void RemovePoint(const TSplineBaseControlPoint<Dim, 3>& TargetPointStruct) override;

	virtual bool AdjustCtrlPointPos(TSplineBaseControlPoint<Dim, 3>& PointStructToAdjust, const TVectorX<Dim>& To, int32 TangentFlag = 0, int32 NthPointOfFrom = 0) override;

	virtual bool AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 TangentFlag = 0, int32 NthPointOfFrom = 0, double ToleranceSqr = 1.) override;

	virtual void Reverse() override;

	virtual TVectorX<Dim> GetPosition(double T) const override;

	virtual TVectorX<Dim> GetTangent(double T) const override;

	virtual double GetPlanCurvature(double T, int32 PlanIndex = 0) const override;

	virtual double GetCurvature(double T) const override;

	virtual void ToPolynomialForm(TArray<TArray<TVectorX<Dim+1> > >& OutPolyForms) const override;

	virtual TTuple<double, double> GetParamRange() const override;

	virtual bool FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr = 1.) const override;

protected:
	TDoubleLinkedList<FControlPointTypeRef> CtrlPointsList;

	double GetNormalizedParam(const FPointNode* StartNode, const FPointNode* EndNode, double T) const;

	TBezierCurve<Dim, 3> MakeBezierCurve(const FPointNode* StartNode, const FPointNode* EndNode) const;

	void UpdateBezierString(FPointNode* NodeToUpdateFirst = nullptr);

	bool AdjustPointByStaticPointReturnShouldSpread(FPointNode* Node, bool bFromNext = true);
};

template<int32 Dim>
struct TSplineTraitByType<ESplineType::BezierString, Dim, 3>
{
	using FSplineType = typename TBezierString3<Dim>;
	using FControlPointType = typename TBezierString3ControlPoint<Dim>;
};

#include "BezierString.inl"
