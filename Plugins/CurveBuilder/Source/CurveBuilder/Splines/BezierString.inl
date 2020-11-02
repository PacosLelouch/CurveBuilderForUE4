// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "BezierString.h"

template<int32 Dim>
inline TBezierString3<Dim>::TBezierString3(const TBezierString3<Dim>& InSpline)
{
	for (const auto& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(Pos);
	}
}

template<int32 Dim>
inline TBezierString3<Dim>& TBezierString3<Dim>::operator=(const TBezierString3<Dim>& InSpline)
{
	for (const auto& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(Pos);
	}
	return *this;
}

template<int32 Dim>
inline typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeByParam(double Param, int32 NthNode) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (FMath::IsNearlyEqual(Node->GetValue().Param, Param)) {
			if (Count == NthNode) {
				return Node;
			}
			++Count;
		}
		Node = Node->GetNextNode();
	}
	return nullptr;
}

template<int32 Dim>
inline typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if ((TVecLib<Dim+1>::Projection(Node->GetValue().Pos) - Point).IsNearlyZero()) {
			if (Count == NthNode) {
				return Node;
			}
			++Count;
		}
		Node = Node->GetNextNode();
	}
	return nullptr;
}

template<int32 Dim>
inline void TBezierString3<Dim>::GetBezierCurves(TArray<TBezierCurve<Dim, 3> >& BezierCurves, TArray<TTuple<double, double> >& ParamRanges) const
{
	FPointNode* Node = CtrlPointsList.GetHead();
	if (!Node) {
		return;
	}
	BezierCurves.Empty(CtrlPointsList.Num() - 1);
	ParamRanges.Empty(CtrlPointsList.Num() - 1);
	FPointNode* NextNode = Node->GetNextNode();
	while (Node && NextNode) {
		const auto& NodeVal = Node->GetValue();
		const auto& NextNodeVal = NextNode->GetValue();
		BezierCurves.Emplace({ NodeVal.Pos, NodeVal.NextCtrlPointPos, NextNodeVal.PrevCtrlPointPos, NextNodeVal.Pos });
		ParamRanges.Emplace(MakeTuple(NodeVal.Param, NextNodeVal.Param));
		Node = Node->GetNextNode();
		NextNode = Node->GetNextNode();
	}
}

template<int32 Dim>
inline void TBezierString3<Dim>::Split(TBezierString3<Dim>& OutFirst, TBezierString3<Dim>& OutSecond, double T)
{
	OutFirst.Reset();
	OutSecond.Reset();
	TTuple<double, double> ParamRange = GetParamRange();
	if (T >= ParamRange.Get<1>() || T <= ParamRange.Get<0>()) {
		return;
	}

	FPointNode* Node = CtrlPointsList.GetHead();
	TArray<TVectorX<Dim+1> > SplitCurveCtrlPoints;
	SplitCurveCtrlPoints.Reserve(4);
	double SplitT = -1.;
	while (Node) {
		if (Node->GetValue().Param < T) {
			auto Val = Node->GetValue();
			Val.Param = static_cast<double>(OutFirst.GetCtrlPointNum());
			OutFirst.AddPointAtLast(FirstVal);
			FPointNode* NextNode = Node->GetNextNode();
			if (NextNode && NextNode->GetValue().Param >= T) {
				SplitCurveCtrlPoints.Add(Val.Pos);
				SplitCurveCtrlPoints.Add(Val.NextCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValue().PrevCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValue().Pos);
				SplitT = (T - Node->GetValue().Param) / (NextNode->GetValue().Param - Node->GetValue().Param);
			}
		}
		else {
			auto Val = Node->GetValue();
			Val.Param = static_cast<double>(OutSecond.GetCtrlPointNum() + 1);
			OutSecond.AddPointAtLast(Node->GetValue());
		}
		Node = Node->GetNextNode();
	}
	if (SplitCurveCtrlPoints.Num() != 4) {
		return;
	}

	TBezierCurve<Dim, 3> NewLeft, NewRight;
	TBezierCurve<Dim, 3> SplitCurve(SplitCurveCtrlPoints);
	SplitCurve.Split(NewLeft, NewRight, SplitT);

	OutFirst.LastNode()->GetValue().NextCtrlPointPos = NewLeft.GetPointHomogeneous(1);
	OutSecond.FirstNode()->GetValue().PrevCtrlPointPos = NewRight.GetPointHomogeneous(2);

	TBezierString3ControlPoint<Dim> Val(
		SplitPos, 
		NewLeft.GetPointHomogeneous(2),
		NewRight.GetPointHomogeneous(1),
		static_cast<double>(OutFirst.GetCtrlPointNum()));
	OutFirst.AddPointAtLast(Val);
	Val.Param = 0.;
	OutSecond.AddPointAt(Val, 0);
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointWithParamWithoutChangingShape(double Param)
{
	//TODO
}


template<int32 Dim>
inline TTuple<double, double> TBezierString3<Dim>::GetParamRange() const
{
	if (CtrlPointsList.GetHead() && CtrlPointsList.GetTail()) {
		return MakeTuple(CtrlPointsList.GetHead()->GetValue().Param, CtrlPointsList.GetTail()->GetValue().Param);
	}
	return MakeTuple(0., 0.);
}
