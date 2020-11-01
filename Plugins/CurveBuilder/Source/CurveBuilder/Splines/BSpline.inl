// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "BSpline.h"


template<int32 Dim, int32 Degree>
inline TClampedBSpline<Dim, Degree>::TClampedBSpline(const TClampedBSpline<Dim, Degree>& InSpline)
{
	for (const auto& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(Pos);
	}
}

template<int32 Dim, int32 Degree>
inline typename TClampedBSpline<Dim, Degree>::FPointNode* TClampedBSpline<Dim, Degree>::FindNodeByParam(double Param, int32 NthNode) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (Node->GetValue().Param == Param) {
			if (Count == NthNode) {
				return Node;
			}
			++Count;
		}
		Node = Node->GetNextNode();
	}
	return nullptr;
}

template<int32 Dim, int32 Degree>
inline typename TClampedBSpline<Dim, Degree>::FPointNode* TClampedBSpline<Dim, Degree>::FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (TVecLib<Dim+1>::Projection(Node->GetValue().Pos) == Point) {
			if (Count == NthNode) {
				return Node;
			}
			++Count;
		}
		Node = Node->GetNextNode();
	}
	return nullptr;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetOpenFormPointsAndParams(TArray<TVectorX<Dim+1> >& CtrlPoints, TArray<double>& Params) const
{
	int32 ListNum = CtrlPointsList.Num();
	int32 ExtraNum = (Degree - 1) << 1;
	CtrlPoints.Reserve(ListNum + ExtraNum);
	Params.Reserve(ListNum + ExtraNum);
	//int32 Index = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	for (int32 i = 1; i < Degree; ++i) {
		CtrlPoints.Add(Node->GetValue().Pos);
		Params.Add(Node->GetValue().Param);
		//Params.Add(0.);
	}
	while (Node) {
		CtrlPoints.Add(Node->GetValue().Pos);
		Params.Add(Node->GetValue().Param);
		//Params.Add(static_cast<double>(Index));
		Node = Node->GetNextNode();
		//++Index;
	}
	if (ListNum > 1) {
		for (int32 i = 1; i < Degree; ++i) {
			CtrlPoints.Add(CtrlPointsList.GetTail()->GetValue().Pos);
			Params.Add(CtrlPointsList.GetTail()->GetValue().Param);
			//Params.Add(static_cast<double>(Index - 1));
		}
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::CreateHodograph(TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const
{
	OutHodograph.Reset();
	FPointNode* CurNode = CtrlPointsList.GetHead();
	if (!CurNode) {
		return;
	}
	FPointNode* NextNode = CurNode->GetNextNode();
	double CurParam = CurNode->GetValue().Param;

	TArray<TVectorX<Dim+1> > CtrlPoints;
	TArray<double> Params;
	GetOpenFormPointsAndParams(CtrlPoints, Params);
	constexpr auto DegreeDbl = static_cast<double>(Degree);

	for (int32 i = Degree - 1; i + Degree < Params.Num(); ++i) {
		TVectorX<Dim> DiffPos = TVecLib<Dim+1>::Projection(CtrlPoints[i + 1]) - TVecLib<Dim+1>::Projection(CtrlPoints[i]);
		double WN = TVecLib<Dim+1>::Last(CtrlPoints[i + 1]), WC = TVecLib<Dim+1>::Last(CtrlPoints[i]);
		double Weight = FMath::IsNearlyZero(WC) ? 1. : WN / WC;
		double DiffParam = Params[i + Degree] - Params[i];
		//	// H_i = d * \frac{P_{i+1} - P_i}{t_{i+d} - t_i}.
		OutHodograph.AddPointAtLast(FMath::IsNearlyZero(DiffParam) ? DiffPos : DiffPos * DegreeDbl / DiffParam, Params[i], Weight);
	}

	//while (NextNode) {
	//	// H_i = n * \frac{P_{i+1} - P_i}{t_{i+n} - t_i}. Where t_{i+n} - t_i = n
	//	const auto& NextPoint = NextNode->GetValue();
	//	const auto& CurPoint = CurNode->GetValue();
	//	double WN = TVecLib<Dim+1>::Last(NextPoint.Pos), WC = TVecLib<Dim+1>::Last(CurPoint.Pos);
	//	double Weight = FMath::IsNearlyZero(WC) ? 1. : WN / WC;
	//	TVectorX<Dim> DiffPos = TVecLib<Dim+1>::Projection(NextPoint.Pos) - TVecLib<Dim+1>::Projection(CurPoint.Pos);

	//	OutHodograph.AddPointAtLast(DiffPos, CurParam, Weight);
	//	CurParam += NextPoint.Param - CurPoint.Param;
	//	CurNode = NextNode;
	//	NextNode = NextNode->GetNextNode();
	//}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::Split(TClampedBSpline<Dim, Degree>& OutFirst, TClampedBSpline<Dim, Degree>& OutSecond, double T)
{
	OutFirst.Reset();
	OutSecond.Reset();
	TVectorX<Dim> SplitPoint = GetPosition(T);
	TVectorX<Dim+1> SplitPointHomogeneous = TVecLib<Dim>::Homogeneous(SplitPoint, 1.);
	TClampedBSplineControlPoint<Dim> NewPoint(SplitPointHomogeneous, T);

	OutSecond.AddPointAtLast(NewPoint);
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (Node->GetValue().Param < Node->GetValue().Param) {
			OutFirst.AddPointAtLast(Node->GetValue());
		}
		else {
			OutSecond.AddPointAtLast(Node->GetValue());
		}
		Node = Node->GetNextNode();
	}
	OutFirst.AddPointAtLast(NewPoint);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtLast(const TClampedBSplineControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddTail(PointStruct);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAt(const TClampedBSplineControlPoint<Dim>& PointStruct, int32 Index)
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

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointWithParamWithoutChangingShape(double Param)
{
	//TODO
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddTail(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam));
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param, int32 Index, double Weight)
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
		FPointNode* BeforeNodeToInsertBefore = CtrlPointsList.GetHead();
		if (BeforeNodeToInsertBefore == NodeToInsertBefore) {
			double InParam = Param ? Param.GetValue() : GetParamRange().Get<0>() - 1.;
			CtrlPointsList.AddHead(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam));
		}
		else {
			while (BeforeNodeToInsertBefore->GetNextNode() != NodeToInsertBefore) {
				BeforeNodeToInsertBefore = BeforeNodeToInsertBefore->GetNextNode();
			}
			double InParam = Param ? Param.GetValue() : 
				(NodeToInsertBefore->GetValue().Param + BeforeNodeToInsertBefore->GetValue().Param) * 0.5;
			CtrlPointsList.InsertNode(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam), NodeToInsertBefore);
		}
	}
	else {
		double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
		CtrlPointsList.AddTail(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam));
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::RemovePointAt(int32 Index)
{
	FPointNode* Node = CtrlPointsList.GetHead();
	for (int32 i = 0; i < Index; ++i) {
		if (Node) {
			Node = Node->GetNextNode();
		}
	}
	CtrlPointsList.RemoveNode(Node);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByPosition(Point, NthPointOfFrom);
	CtrlPointsList.RemoveNode(Node);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::RemovePoint(double Param, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(Param, NthPointOfFrom);
	CtrlPointsList.RemoveNode(Node);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByPosition(From, NthPointOfFrom);
	Node->GetValue().Pos = TVecLib<Dim>::Homogeneous(To, 1.);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(From, NthPointOfFrom);
	Node->GetValue().Param = To;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::Reverse()
{
	TDoubleLinkedList<TClampedBSplineControlPoint<Dim> > NewList;
	for (const auto& Point : CtrlPointsList) {
		NewList.AddHead(Point);
	}
	CtrlPointsList.Empty();
	for (const auto& Point : NewList) {
		CtrlPointsList.AddTail(Point);
	}
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TClampedBSpline<Dim, Degree>::GetPosition(double T) const
{
	int32 ListNum = CtrlPointsList.Num();
	if (ListNum == 0) {
		return TVecLib<Dim>::Zero();
	}
	else if (ListNum == 1) {
		return TVecLib<Dim+1>::Projection(CtrlPointsList.GetHead()->GetValue().Pos);
	}
	TArray<TVectorX<Dim+1> > CtrlPoints;
	TArray<double> Params;
	GetOpenFormPointsAndParams(CtrlPoints, Params);

	return DeBoorAlgorithm(T, CtrlPoints, Params);
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TClampedBSpline<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 1)) {
		if (CtrlPointsList.Num() > 1) {
			FPointNode* Head = CtrlPointsList.GetHead();
			return TVecLib<Dim+1>::Projection(Head->GetNextNode()->GetValue().Pos)
				- TVecLib<Dim+1>::Projection(Head->GetValue().Pos);
		}
		else {
			return TVecLib<Dim>::Zero();
		}
	}
	TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);
	TTuple<double, double> ParamRange = GetParamRange();
	TTuple<double, double> HParamRange = Hodograph.GetParamRange();
	double TH = ConvertRange(T, ParamRange, HParamRange);
	return Hodograph.GetPosition(TH);
}

template<int32 Dim, int32 Degree>
inline double TClampedBSpline<Dim, Degree>::GetPrincipalCurvature(double T, int32 Principal) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TClampedBSpline<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	TTuple<double, double> ParamRange = GetParamRange();
	TTuple<double, double> HParamRange = Hodograph.GetParamRange();
	TTuple<double, double> H2ParamRange = Hodograph2.GetParamRange();
	double TH = ConvertRange(T, ParamRange, HParamRange);
	double TH2 = ConvertRange(T, ParamRange, H2ParamRange);
	return TVecLib<Dim>::PrincipalCurvature(Hodograph.GetPosition(TH), Hodograph2.GetPosition(TH2), Principal);
}

template<int32 Dim, int32 Degree>
inline double TClampedBSpline<Dim, Degree>::GetCurvature(double T) const
{
	if (constexpr(Degree <= 1)) {
		return 0.0;
	}
	TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)> Hodograph;
	CreateHodograph(Hodograph);

	TClampedBSpline<Dim, CLAMP_DEGREE(Degree-2, 0)> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	TTuple<double, double> ParamRange = GetParamRange();
	TTuple<double, double> HParamRange = Hodograph.GetParamRange();
	TTuple<double, double> H2ParamRange = Hodograph2.GetParamRange();
	double TH = ConvertRange(T, ParamRange, HParamRange);
	double TH2 = ConvertRange(T, ParamRange, H2ParamRange);
	return TVecLib<Dim>::Curvature(Hodograph.GetPosition(TH), Hodograph2.GetPosition(TH2));
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::ToPolynomialForm(TArray<TArray<TVectorX<Dim+1>>>& OutPolyForms) const
{
	//TODO
}

template<int32 Dim, int32 Degree>
inline TTuple<double, double> TClampedBSpline<Dim, Degree>::GetParamRange() const
{
	if (CtrlPointsList.GetHead() && CtrlPointsList.GetTail()) {
		return MakeTuple(CtrlPointsList.GetHead()->GetValue().Param, CtrlPointsList.GetTail()->GetValue().Param);
	}
	return MakeTuple(0., 0.);
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TClampedBSpline<Dim, Degree>::DeBoorAlgorithm(double T, const TArray<TVectorX<Dim+1>>& CtrlPoints, const TArray<double>& Params) const
{
	int32 ListNum = CtrlPointsList.Num();
	TArray<double> N;
	N.SetNumZeroed(CtrlPoints.Num() - 1);
	int32 StartI = N.Num();
	for (int32 i = 0; i < N.Num(); ++i) {
		if (Params[i] <= T && T <= Params[i + 1]) {
			if (StartI == N.Num()) {
				StartI = i;
			}
			N[i] = 1.;
		}
	}
	for (int32 k = 1; k <= Degree; ++k) {
		for (int32 i = 0; i + k < N.Num(); ++i) {
			double De0 = Params[i + k - 1] - Params[i];
			double De1 = Params[i + k] - Params[i + 1];
			double W0 = FMath::IsNearlyZero(De0) ? 0. : (T - Params[i]) / De0;
			double W1 = FMath::IsNearlyZero(De1) ? 1. : 1. - (T - Params[i + 1]) / De1;
			N[i] = W0 * N[i] + W1 * N[i + 1];
		}
	}
	TVectorX<Dim> Position = TVecLib<Dim>::Zero();
	if (StartI < N.Num()) {
		for (int32 i = StartI; i <= Degree + StartI; ++i) {
			Position += CtrlPoints[i + Degree - 1] * N[i];
		}
	}
	return Position;
}
