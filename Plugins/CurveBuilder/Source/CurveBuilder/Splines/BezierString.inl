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
inline TBezierString3<Dim>::TBezierString3(const TArray<TBezierCurve<Dim, 3>>& InCurves)
{
	FromCurveArray(InCurves);
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
inline void TBezierString3<Dim>::FromCurveArray(const TArray<TBezierCurve<Dim, 3>>& InCurves)
{
	for (int32 i = 0; i < InCurves.Num(); ++i) {
		CtrlPointsList.AddTail(TBezierString3ControlPoint<Dim>(
			InCurves[i].GetPointHomogeneous(0),
			TVecLib<Dim+1>::Zero(),
			InCurves[i].GetPointHomogeneous(1),
			static_cast<double>(i)));
		if (i > 0) {
			if (!(InCurves[i].GetPointHomogeneous(0) - InCurves[i - 1].GetPointHomogeneous(3)).IsNearlyZero()) {
				Reset();
				return;
			}
			CtrlPointsList.GetTail()->GetValue().PrevCtrlPointPos = InCurves[i - 1].GetPointHomogeneous(3);
		}
	}
	if (InCurves.Num() > 0) {
		CtrlPointsList.AddTail(TBezierString3ControlPoint<Dim>(
			InCurves[InCurves.Num() - 1].GetPointHomogeneous(3),
			InCurves[i].GetPointHomogeneous(2),
			TVecLib<Dim+1>::Zero(),
			static_cast<double>(InCurves.Num())));
	}
}

template<int32 Dim>
inline typename typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeByParam(double Param, int32 NthNode) const
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
inline typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeGreaterThanParam(double Param, int32 NthNode) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (Node->GetValue().Param > Param) {
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
inline typename typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode) const
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
inline void TBezierString3<Dim>::Split(TBezierString3<Dim>& OutFirst, TBezierString3<Dim>& OutSecond, double T) const
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
		if (Node->GetValue().Param <= T) {
			auto Val = Node->GetValue();
			Val.Param = static_cast<double>(OutFirst.GetCtrlPointNum());
			OutFirst.AddPointAtLast(Val);
			FPointNode* NextNode = Node->GetNextNode();
			if (NextNode && T < NextNode->GetValue().Param) {
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
inline void TBezierString3<Dim>::AddPointAtLast(const TBezierString3ControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddTail(PointStruct);
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAtFirst(const TBezierString3ControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddHead(PointStruct);
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAt(const TBezierString3ControlPoint<Dim>& PointStruct, int32 Index)
{
	FPointNode* NodeToInsertBefore = CtrlPointsList.GetHead();
	for (int32 i = 0; i < Index; ++i) {
		if (NodeToInsertBefore) {
			NodeToInsertBefore = NodeToInsertBefore->GetNextNode();
		}
		else {
			break;
		}
	}
	if (NodeToInsertBefore) {
		CtrlPointsList.InsertNode(PointStruct, NodeToInsertBefore);
	}
	else {
		CtrlPointsList.AddTail(PointStruct);
	}
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointWithParamWithoutChangingShape(double T)
{
	TTuple<double, double> ParamRange = GetParamRange();
	if (T >= ParamRange.Get<1>() || T <= ParamRange.Get<0>()) {
		return;
	}

	FPointNode* Node = CtrlPointsList.GetHead();
	FPointNode* NodeToInsertBefore = nullptr;
	TArray<TVectorX<Dim+1> > SplitCurveCtrlPoints;
	SplitCurveCtrlPoints.Reserve(4);
	double SplitT = -1.;
	while (Node) {
		if (Node->GetValue().Param <= T) {
			auto Val = Node->GetValue();
			Val.Param = static_cast<double>(OutFirst.GetCtrlPointNum());
			OutFirst.AddPointAtLast(Val);
			FPointNode* NextNode = Node->GetNextNode();
			if (NextNode && T < NextNode->GetValue().Param) {
				NodeToInsertBefore = NextNode;
				SplitCurveCtrlPoints.Add(Val.Pos);
				SplitCurveCtrlPoints.Add(Val.NextCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValue().PrevCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValue().Pos);
				//SplitT = (T - Node->GetValue().Param) / (NextNode->GetValue().Param - Node->GetValue().Param);
				break;
			}
		}
		//else {
		//	auto Val = Node->GetValue();
		//	Val.Param = static_cast<double>(OutSecond.GetCtrlPointNum() + 1);
		//	OutSecond.AddPointAtLast(Node->GetValue());
		//}
		Node = Node->GetNextNode();
	}
	if (SplitCurveCtrlPoints.Num() != 4 || !NodeToInsertBefore) {
		return;
	}

	TBezierCurve<Dim, 3> NewLeft, NewRight;
	TBezierCurve<Dim, 3> SplitCurve(SplitCurveCtrlPoints);
	SplitCurve.Split(NewLeft, NewRight, SplitT);

	FPointNode* NodeToInsertAfter = NodeToInsertBefore->GetPrevNode();

	NodeToInsertAfter->GetValue().NextCtrlPointPos = NewLeft.GetPointHomogeneous(1);
	NodeToInsertBefore->GetValue().PrevCtrlPointPos = NewRight.GetPointHomogeneous(2);

	TBezierString3ControlPoint<Dim> Val(
		SplitPos,
		NewLeft.GetPointHomogeneous(2),
		NewRight.GetPointHomogeneous(1),
		T);
	CtrlPointsList.InsertNode(Val, NodeToInsertBefore);
}

template<int32 Dim>
inline void TBezierString3<Dim>::AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(From, NthPointOfFrom);
	Node->GetValue().Param = To;
	UpdateBezierString(Node);
}

template<int32 Dim>
inline void TBezierString3<Dim>::RemovePoint(double Param, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(Param, NthPointOfFrom);
	CtrlPointsList.RemoveNode(Node);
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddTail(TBezierString3ControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam));
	UpdateBezierString(CtrlPointsList.GetTail());
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAtHead(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddHead(TBezierString3ControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam));
	UpdateBezierString(CtrlPointsList.GetHead());
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param, int32 Index, double Weight)
{
	double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	TBezierString3ControlPoint<Dim> PointStruct(TVecLib<Dim>::Homogeneous(Point, Weight), InParam);

	FPointNode* NodeToInsertBefore = CtrlPointsList.GetHead();
	for (int32 i = 0; i < Index; ++i) {
		if (NodeToInsertBefore) {
			NodeToInsertBefore = NodeToInsertBefore->GetNextNode();
		}
		else {
			break;
		}
	}
	if (NodeToInsertBefore) {
		CtrlPointsList.InsertNode(PointStruct, NodeToInsertBefore);
		UpdateBezierString(NodeToInsertBefore->GetPrevNode());
	}
	else {
		CtrlPointsList.AddTail(PointStruct);
		UpdateBezierString(CtrlPointsList.GetTail());
	}
}

template<int32 Dim>
inline void TBezierString3<Dim>::RemovePointAt(int32 Index)
{
	FPointNode* Node = CtrlPointsList.GetHead();
	for (int32 i = 0; i < Index; ++i) {
		if (Node) {
			Node = Node->GetNextNode();
		}
	}
	CtrlPointsList.RemoveNode(Node);
}

template<int32 Dim>
inline void TBezierString3<Dim>::RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByPosition(Point, NthPointOfFrom);
	CtrlPointsList.RemoveNode(Node);
}

template<int32 Dim>
inline void TBezierString3<Dim>::AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByPosition(From, NthPointOfFrom);
	Node->GetValue().Pos = TVecLib<Dim>::Homogeneous(To, 1.);
	UpdateBezierString(Node);
}

template<int32 Dim>
inline void TBezierString3<Dim>::Reverse()
{
	if (!CtrlPointsList.GetHead() || !CtrlPointsList.GetTail()) {
		return;
	}
	TDoubleLinkedList<TBezierString3ControlPoint<Dim> > NewList;
	double SumParam = CtrlPointsList.GetHead()->GetValue().Param + CtrlPointsList.GetTail()->GetValue().Param;
	for (const auto& Point : CtrlPointsList) {
		NewList.AddHead(Point);
		NewList->GetHead()->GetValue().Param = SumParam - NewList->GetHead()->GetValue().Param;
		std::swap(NewList->GetHead()->GetValue().PrevCtrlPointPos, NewList->GetHead()->GetValue().NextCtrlPointPos);
	}
	CtrlPointsList.Empty();
	for (const auto& Point : NewList) {
		CtrlPointsList.AddTail(Point);
	}
}

template<int32 Dim>
inline TVectorX<Dim> TBezierString3<Dim>::GetPosition(double T) const
{
	int32 ListNum = CtrlPointsList.Num();
	if (ListNum == 0) {
		return TVecLib<Dim>::Zero();
	}
	else if (ListNum == 1) {
		return TVecLib<Dim+1>::Projection(CtrlPointsList.GetHead()->GetValue().Pos);
	}
	const auto& ParamRange = GetParamRange();
	if (ParamRange.Get<0>() >= T) {
		TVecLib<Dim+1>::Projection(CtrlPointsList.GetHead()->GetValue().Pos);
	}
	else if(T >= ParamRange.Get<1>()) {
		return TVecLib<Dim+1>::Projection(CtrlPointsList.GetTail()->GetValue().Pos);
	}

	FPointNode* EndNode = FindNodeGreaterThanParam(T);
	FPointNode* StartNode = EndNode->GetPrevNode();
	double TN = GetNormalizedParam(StartNode, EndNode, T);
	TBezierCurve<Dim, 3> TargetCurve = MakeBezierCurve(StartNode, EndNode);
	return TargetCurve.GetPosition(TN);
}

template<int32 Dim>
inline TVectorX<Dim> TBezierString3<Dim>::GetTangent(double T) const
{
	//if (constexpr(Degree <= 0)) {
	//	return TVecLib<Dim>::Zero();
	//}
	FPointNode* EndNode = FindNodeGreaterThanParam(T);
	FPointNode* StartNode = EndNode->GetPrevNode();
	double TN = GetNormalizedParam(StartNode, EndNode, T);
	TBezierCurve<Dim, 3> Curve = MakeBezierCurve(StartNode, EndNode);
	TBezierCurve<Dim, 2> Hodograph;
	Curve.CreateHodograph(Hodograph);
	
	TVectorX<Dim> Tangent = Hodograph.GetPosition(TN);
	return Tangent.IsNearlyZero() ? Hodograph.GetTangent(TN) : Tangent;
}

template<int32 Dim>
inline double TBezierString3<Dim>::GetPrincipalCurvature(double T, int32 Principal) const
{
	FPointNode* EndNode = FindNodeGreaterThanParam(T);
	FPointNode* StartNode = EndNode->GetPrevNode();
	double TN = GetNormalizedParam(StartNode, EndNode, T);
	TBezierCurve<Dim, 3> Curve = MakeBezierCurve(StartNode, EndNode);
	TBezierCurve<Dim, 2> Hodograph;
	Curve.CreateHodograph(Hodograph);
	TBezierCurve<Dim, 1> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::PrincipalCurvature(Hodograph.GetPosition(TN), Hodograph2.GetPosition(TN), Principal);
}

template<int32 Dim>
inline double TBezierString3<Dim>::GetCurvature(double T) const
{
	FPointNode* EndNode = FindNodeGreaterThanParam(T);
	FPointNode* StartNode = EndNode->GetPrevNode();
	double TN = GetNormalizedParam(StartNode, EndNode, T);
	TBezierCurve<Dim, 3> Curve = MakeBezierCurve(StartNode, EndNode);
	TBezierCurve<Dim, 2> Hodograph;
	Curve.CreateHodograph(Hodograph);
	TBezierCurve<Dim, 1> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::Curvature(Hodograph.GetPosition(TN), Hodograph2.GetPosition(TN));
}

template<int32 Dim>
inline void TBezierString3<Dim>::ToPolynomialForm(TArray<TArray<TVectorX<Dim+1>>>& OutPolyForms) const
{
	FPointNode* Node = CtrlPointsList.GetHead();
	if (!Node) {
		return;
	}
	OutPolyForms.Empty(CtrlPointsList.Num() - 1);
	while (Node && Node->GetNextNode()) {
		FPointNode* Next = Node->GetNextNode();

		TArray<TVectorX<Dim+1> >& NewArray = OutPolyForms.AddDefaulted_GetRef();
		NewArray.SetNum(4);
		TBezierCurve<Dim, 3> NewBezier = MakeBezierCurve(Node, Next);
		NewBezier.ToPolynomialForm(NewArray.GetData());

		Node = Next;
	}
}


template<int32 Dim>
inline TTuple<double, double> TBezierString3<Dim>::GetParamRange() const
{
	if (CtrlPointsList.GetHead() && CtrlPointsList.GetTail()) {
		return MakeTuple(CtrlPointsList.GetHead()->GetValue().Param, CtrlPointsList.GetTail()->GetValue().Param);
	}
	return MakeTuple(0., 0.);
}

template<int32 Dim>
inline bool TBezierString3<Dim>::FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr) const
{
	FPointNode* Node = CtrlPointsList.GetHead();
	if (!Node) {
		return false;
	}
	TOptional<double> CurParam;
	TOptional<double> CurDistSqr;
	while (Node && Node->GetNextNode()) {
		FPointNode* Next = Node->GetNextNode();

		TBezierCurve<Dim, 3> NewBezier = MakeBezierCurve(Node, Next);
		double NewParamNormal = -1.;
		if (NewBezier.FindParamByPosition(NewParamNormal, InPos, ToleranceSqr)) {
			double NewParam = Node->GetValue().Param * (1. - NewParamNormal) + Next->GetValue().Param * NewParamNormal;
			if (CurParam) {
				TVectorX<Dim> NewPos = NewBezier.GetPosition(NewParamNormal);
				double NewDistSqr = TVecLib<Dim>::SizeSquared(NewPos - InPos);
				if (NewDistSqr < CurDistSqr.GetValue()) {
					CurDistSqr = NewDistSqr;
					CurParam = NewParam;
				}

			}
			else {
				TVectorX<Dim> CurPos = NewBezier.GetPosition(NewParamNormal);
				CurDistSqr = TVecLib<Dim>::SizeSquared(CurPos - InPos);
				CurParam = NewParam;
			}
		}

		Node = Next;
	}

	if (CurParam) {
		OutParam = CurParam.GetValue();
		return true;
	}
	return false;
}

template<int32 Dim>
inline double TBezierString3<Dim>::GetNormalizedParam(
	const typename TBezierString3<Dim>::FPointNode* StartNode,
	const typename TBezierString3<Dim>::FPointNode* EndNode, 
	double T) const
{
	double De = EndNode->GetValue().Param - StartNode->GetValue().Param;
	return FMath::IsNearlyZero(De) ? 0.5 : (T - StartNode->GetValue().Param) / De;
}

template<int32 Dim>
inline TBezierCurve<Dim, 3> TBezierString3<Dim>::MakeBezierCurve(
	const typename TBezierString3<Dim>::FPointNode* StartNode, 
	const typename TBezierString3<Dim>::FPointNode* EndNode) const
{
	return MoveTemp(TBezierCurve<Dim, 3>({
		StartNode->GetValue().Pos,
		StartNode->GetValue().NextCtrlPointPos,
		EndNode->GetValue().PrevCtrlPointPos,
		EndNode->GetValue().Pos }));
}

template<int32 Dim>
inline void TBezierString3<Dim>::UpdateBezierString(typename TBezierString3<Dim>::FPointNode* NodeToUpdateFirst)
{
	if (!NodeToUpdateFirst) {
		//TODO: Interpolate all
	}
	//TODO
}
