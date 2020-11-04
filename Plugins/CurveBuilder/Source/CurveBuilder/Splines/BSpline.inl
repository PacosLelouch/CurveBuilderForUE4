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
	for (const auto& P : InSpline.KnotIntervals) {
		KnotIntervals.Add(P);
	}
}

template<int32 Dim, int32 Degree>
inline TClampedBSpline<Dim, Degree>& TClampedBSpline<Dim, Degree>::operator=(const TClampedBSpline<Dim, Degree>& InSpline)
{
	CtrlPointsList.Empty();
	KnotIntervals.Empty(InSpline.KnotIntervals.Num());
	for (const auto& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(Pos);
	}
	for (const auto& P : InSpline.KnotIntervals) {
		KnotIntervals.Add(P);
	}
	return *this;
}

//template<int32 Dim, int32 Degree>
//inline typename TClampedBSpline<Dim, Degree>::FPointNode* TClampedBSpline<Dim, Degree>::FindNodeByParam(double Param, int32 NthNode) const
//{
//	int32 Count = 0;
//	FPointNode* Node = CtrlPointsList.GetHead();
//	while (Node) {
//		if (FMath::IsNearlyEqual(Node->GetValue().Param, Param)) {
//			if (Count == NthNode) {
//				return Node;
//			}
//			++Count;
//		}
//		Node = Node->GetNextNode();
//	}
//	return nullptr;
//}

template<int32 Dim, int32 Degree>
inline typename TClampedBSpline<Dim, Degree>::FPointNode* TClampedBSpline<Dim, Degree>::FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode) const
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

//template<int32 Dim, int32 Degree>
//inline void TClampedBSpline<Dim, Degree>::GetOpenFormPointsAndParams(TArray<TVectorX<Dim+1> >& CtrlPoints, TArray<double>& Params) const
//{
//	int32 ListNum = CtrlPointsList.Num();
//	static constexpr int32 RepeatNum = Degree;
//	//static constexpr int32 ExtraNum = RepeatNum << 1;
//	//CtrlPoints.Reserve(ListNum + ExtraNum);
//	//Params.Reserve(ListNum + ExtraNum);
//	//int32 Index = 0;
//	CtrlPoints.Empty(ListNum * (RepeatNum + 1));
//	Params.Empty(ListNum * (RepeatNum + 1));
//	FPointNode* Node = CtrlPointsList.GetHead();
//	if (!Node) {
//		return;
//	}
//	for (int32 i = 1; i <= RepeatNum; ++i) {
//		CtrlPoints.Add(Node->GetValue().Pos);
//		Params.Add(Node->GetValue().Param);
//		//Params.Add(0.);
//	}
//	while (Node) {
//		for (int32 n = 0; n <= Node->GetValue().MiddleRepeatNum; ++n) {
//			CtrlPoints.Add(Node->GetValue().Pos);
//			Params.Add(Node->GetValue().Param);
//			//Params.Add(static_cast<double>(Index));
//		}
//		Node = Node->GetNextNode();
//		//++Index;
//	}
//	if (ListNum > 1) {
//		for (int32 i = 1; i <= RepeatNum; ++i) {
//			CtrlPoints.Add(CtrlPointsList.GetTail()->GetValue().Pos);
//			Params.Add(CtrlPointsList.GetTail()->GetValue().Param);
//			//Params.Add(static_cast<double>(Index - 1));
//		}
//	}
//}

//template<int32 Dim, int32 Degree>
//inline void TClampedBSpline<Dim, Degree>::GetCtrlPointsAndParams(TArray<TVectorX<Dim+1>>& CtrlPoints, TArray<double>& Params) const
//{
//	int32 ListNum = CtrlPointsList.Num();
//	static constexpr int32 RepeatNum = Degree;
//	//static constexpr int32 ExtraNum = RepeatNum << 1;
//	//CtrlPoints.Reserve(ListNum + ExtraNum);
//	//Params.Reserve(ListNum + ExtraNum);
//	//int32 Index = 0;
//	CtrlPoints.Empty(ListNum * (RepeatNum + 1));
//	Params.Empty(ListNum * (RepeatNum + 1));
//	FPointNode* Node = CtrlPointsList.GetHead();
//	if (!Node) {
//		return;
//	}
//	while (Node) {
//		for (int32 n = 0; n <= Node->GetValue().MiddleRepeatNum; ++n) {
//			CtrlPoints.Add(Node->GetValue().Pos);
//			Params.Add(Node->GetValue().Param);
//			//Params.Add(static_cast<double>(Index));
//		}
//		Node = Node->GetNextNode();
//		//++Index;
//	}
//}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetCtrlPoints(TArray<TVectorX<Dim+1>>& CtrlPoints) const
{
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		CtrlPoints.Add(Node->GetValue().Pos);
		Node = Node->GetNextNode();
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::ToBezierString(TArray<TBezierCurve<Dim, Degree>>& Beziers) const
{
	// TODO: Fix
	//FPointNode* Node = CtrlPointsList.GetTail();
	//if (!Node) {
	//	return;
	//}
	//Node = Node->GetPrevNode();
	//if (!Node) {
	//	return;
	//}
	FPointNode* Node = CtrlPointsList.GetHead();
	if (!Node) {
		return;
	}
	Node = Node->GetNextNode();
	if (!Node) {
		return;
	}
	Beziers.Empty();
	Node = Node->GetNextNode();

	//TArray<TVectorX<Dim + 1> > EndPoints;
	//EndPoints.Reserve(CtrlPointsList.Num());
	//EndPoints.Add(CtrlPointsList.GetHead()->GetValue().Pos);

	TArray<TVectorX<Dim+1> > CtrlPoints;
	TArray<double> Params;

	TArray<TArray<TVectorX<Dim+1> > > SplitPosArray;
	TArray<TArray<double> > SplitParamArray;

	TClampedBSpline<Dim, Degree> SplitFirst, SplitSecond, ToSplit(*this);
	while (Node && Node->GetNextNode()) {
		double T = Node->GetValue().Param;
		ToSplit.GetOpenFormPointsAndParams(CtrlPoints, Params);
		TVectorX<Dim+1> NewEndPoint = ToSplit.Split(SplitFirst, SplitSecond, T, &SplitPosArray, &SplitParamArray);
		//auto& NewBezier = Beziers.AddDefaulted_GetRef();
		//NewBezier.SetPointHomogeneous(0, EndPoints.Last());
		//for (int32 i = 0; i < SplitPosArray.Num(); ++i) { // TODO: Consider Params
		//	NewBezier.SetPointHomogeneous(i + 1, SplitPosArray[i][0]);
		//}
		TArray<TVectorX<Dim+1> > CurCtrlPoints;
		TArray<double> CurParams;
		SplitFirst.GetCtrlPointsAndParams(CurCtrlPoints, CurParams);
		Beziers.Emplace(CurCtrlPoints.GetData());

		//EndPoints.Add(NewEndPoint);
		Node = Node->GetNextNode();
		if (Node) {
			Node = Node->GetNextNode();
		}
		ToSplit = SplitSecond;
	}
	if (true) {
		TArray<TVectorX<Dim+1> > CurCtrlPoints;
		TArray<double> CurParams;

		const auto& ParamRange = ToSplit.GetParamRange();
		double T = 0.5 * (ParamRange.Get<0>() + ParamRange.Get<1>());
		if (Node) {
			Node = ToSplit.LastNode()->GetPrevNode();
			T = Node->GetValue().Param;
		}

		ToSplit.GetOpenFormPointsAndParams(CtrlPoints, Params);
		TVectorX<Dim+1> NewEndPoint = ToSplit.Split(SplitFirst, SplitSecond, T, &SplitPosArray, &SplitParamArray);
		TVectorX<Dim+1> NewPointsToAdd[] = {
			ToSplit.FirstNode()->GetValue().Pos,
			SplitPosArray[0][0],
			SplitPosArray[0].Last(),
			ToSplit.LastNode()->GetValue().Pos };
		Beziers.Emplace(NewPointsToAdd);
	}
	//auto& NewBezier = Beziers.AddDefaulted_GetRef();
	//for (int32 i = 0; i < SplitPosArray.Num(); ++i) {
	//	NewBezier.SetPointHomogeneous(SplitPosArray.Num() - 1 - i, SplitPosArray[i].Last());
	//}
	//NewBezier.SetPointHomogeneous(SplitPosArray.Num(), CtrlPointsList.GetTail()->GetValue().Pos);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetClampedKnotIntervals(TArray<double>& ClampedKnotIntervals) const
{
	if (KnotIntervals.Num() == 0) {
		return;
	}
	ClampedKnotIntervals.Empty(KnotIntervals.Num() + (Degree << 1));
	for (int32 i = 0; i < Degree; ++i) {
		ClampedKnotIntervals.Add(KnotIntervals[0]);
	}
	for (double KI : KnotIntervals) {
		ClampedKnotIntervals.Add(KI);
	}
	if (KnotIntervals.Num() > 1) {
		for (int32 i = 0; i < Degree; ++i) {
			ClampedKnotIntervals.Add(KnotIntervals.Last());
		}
	}
}

template<int32 Dim, int32 Degree>
inline int32 TClampedBSpline<Dim, Degree>::GetMaxKnotIntervalIndex() const
{
	int32 MaxIndex = 1 + (CtrlPointsList.Num() - 1) / (Degree + 1);
	return FMath::Max(MaxIndex, KnotIntervals.Num() - 1);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddNewKnotIntervalIfNecessary(TOptional<double> Param)
{
	//TODO
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::CreateHodograph(TClampedBSpline<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const
{
	//TODO: Fix
	OutHodograph.Reset();
	FPointNode* CurNode = CtrlPointsList.GetHead();
	if (!CurNode || !CurNode->GetNextNode()) {
		return;
	}
	FPointNode* NextNode = CurNode->GetNextNode();
	double CurParam = CurNode->GetValue().Param;
	//TODO
	TArray<TVectorX<Dim+1> > CtrlPoints;
	GetCtrlPoints(CtrlPoints);
	constexpr auto DegreeDbl = static_cast<double>(Degree);

	for (int32 i = 0; i + Degree + 1 < Params.Num(); ++i) {
		TVectorX<Dim> DiffPos = TVecLib<Dim+1>::Projection(CtrlPoints[i + 1]) - TVecLib<Dim+1>::Projection(CtrlPoints[i]);
		double WN = TVecLib<Dim+1>::Last(CtrlPoints[i + 1]), WC = TVecLib<Dim+1>::Last(CtrlPoints[i]);
		double Weight = FMath::IsNearlyZero(WC) ? 1. : WN / WC;
		//double DiffParam = Params[i + 1] - Params[i];
		double DiffParam = Params[i + Degree + 1] - Params[i + 1];
		// H_i = d * \frac{P_{i+1} - P_i}{t_{i+d} - t_i}?
		OutHodograph.AddPointAtLast(FMath::IsNearlyZero(DiffParam) ? TVecLib<Dim>::Zero() : DiffPos * DegreeDbl / DiffParam, Params[i], Weight);
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
inline TVectorX<Dim+1> TClampedBSpline<Dim, Degree>::Split(
	TClampedBSpline<Dim, Degree>& OutFirst, TClampedBSpline<Dim, Degree>& OutSecond, double T,
	TArray<TArray<TVectorX<Dim+1> > >* SplitPosArray, TArray<TArray<double> >* SplitParamArray) const
{
	//TODO: Fix
	OutFirst.Reset();
	OutSecond.Reset();

	TTuple<double, double> ParamRange = GetParamRange();
	if (T >= ParamRange.Get<1>() || T <= ParamRange.Get<0>()) {
		return TVecLib<Dim+1>::Zero();
	}

	TArray<TVectorX<Dim+1> > CtrlPoints;
	TArray<double> Params;
	GetOpenFormPointsAndParams(CtrlPoints, Params);
	TArray<TArray<TVectorX<Dim+1> > > TempSplitPosArray;
	TArray<TArray<double> > TempSplitParamArray;
	TArray<TArray<TVectorX<Dim+1> > >* SplitPosArrayPtr = SplitPosArray ? SplitPosArray : &TempSplitPosArray;
	TArray<TArray<double> >* SplitParamArrayPtr = SplitParamArray ? SplitParamArray : &TempSplitParamArray;
	TVectorX<Dim+1> ReturnValue = DeBoor(T, CtrlPoints, Params, SplitPosArrayPtr, SplitParamArrayPtr); // ???

	FPointNode* Node = CtrlPointsList.GetHead();

	// Start New

	static constexpr double ErrorTolerance = SMALL_NUMBER;
	TArray<FPointNode*> FirstSplits, SecondSplits;
	FirstSplits.Reserve(CtrlPointsList.Num());
	SecondSplits.Reserve(CtrlPointsList.Num());
	bool bAddTargetAtKnot = false;
	while (Node) {
		if (Node->GetValue().Param < T - ErrorTolerance) {
			FirstSplits.Add(Node);
		}
		else if (T + ErrorTolerance < Node->GetValue().Param) {
			SecondSplits.Add(Node);
		}
		else if (FMath::IsNearlyEqual(T, Node->GetValue().Param, ErrorTolerance)) {
			FirstSplits.Add(Node);
			SecondSplits.Add(Node);
			bAddTargetAtKnot = true;
		}
		Node = Node->GetNextNode();
	}

	for (int32 i = 0; i < FirstSplits.Num() - 1; ++i) {
		OutFirst.AddPointAtLast(FirstSplits[i]->GetValue());
	}
	for (int32 i = 1; i < SecondSplits.Num(); ++i) {
		OutSecond.AddPointAtLast(SecondSplits[i]->GetValue());
	}

	for (int32 i = 0; i < SplitPosArrayPtr->Num() - (bAddTargetAtKnot ? 1 : 0); ++i) {
		OutFirst.AddPointAtLast((*SplitPosArrayPtr)[i][0], (*SplitParamArrayPtr)[i][0]);
	}
	for (int32 i = (bAddTargetAtKnot ? 1 : 0); i < SplitPosArrayPtr->Num(); ++i) {
		OutSecond.AddPointAtFirst((*SplitPosArrayPtr)[i].Last(), (*SplitParamArrayPtr)[i].Last());
	}

	// End New

	//bool bAddTargetAtSecondHead = true;
	//while (Node) {
	//	//if ((*SplitParamArrayPtr)[0].Last() < Node->GetValue().Param && !FMath::IsNearlyEqual((*SplitParamArrayPtr)[0].Last(), Node->GetValue().Param)) {
	//	if ((*SplitParamArrayPtr)[0].Last() <= Node->GetValue().Param) {
	//		OutSecond.AddPointAtLast(Node->GetValue());
	//	}
	//	//else if (Node->GetValue().Param < (*SplitParamArrayPtr)[0][0] && !FMath::IsNearlyEqual((*SplitParamArrayPtr)[0][0], Node->GetValue().Param)) {
	//	else if (Node->GetValue().Param <= (*SplitParamArrayPtr)[0][0]) {
	//		OutFirst.AddPointAtLast(Node->GetValue());
	//	}
	//	else if (FMath::IsNearlyEqual(Node->GetValue().Param, (*SplitParamArrayPtr).Last().Last())) {
	//		bAddTargetAtSecondHead = false;
	//	}
	//	Node = Node->GetNextNode();
	//}

	//for (int32 i = 0; i < SplitPosArrayPtr->Num() - 1 + (bAddTargetAtSecondHead ? 1 : 0); ++i) {
	//	OutFirst.AddPointAtLast((*SplitPosArrayPtr)[i][0], (*SplitParamArrayPtr)[i][0]);
	//}
	//for (int32 i = 1 - (bAddTargetAtSecondHead ? 1 : 0); i < SplitPosArrayPtr->Num(); ++i) {
	//	OutSecond.AddPointAtFirst((*SplitPosArrayPtr)[i].Last(), (*SplitParamArrayPtr)[i].Last());
	//}

	////for (int32 i = 0; i < SplitPosArrayPtr->Num(); ++i) { 
	////	OutFirst.AddPointAtLast((*SplitPosArrayPtr)[i][0], (*SplitParamArrayPtr)[i][0]);
	////	OutSecond.AddPointAtFirst((*SplitPosArrayPtr)[i].Last(), (*SplitParamArrayPtr)[i].Last());
	////}
	return ReturnValue;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtLast(const TClampedBSplineControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddTail(PointStruct);
	AddNewKnotIntervalIfNecessary();
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtFirst(const TClampedBSplineControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddHead(PointStruct);
	AddNewKnotIntervalIfNecessary();
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
	AddNewKnotIntervalIfNecessary();
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointWithParamWithoutChangingShape(double T)
{
	// TODO: Fix
	TTuple<double, double> ParamRange = GetParamRange();
	if (T >= ParamRange.Get<1>() || T <= ParamRange.Get<0>()) {
		return;
	}

	TArray<TVectorX<Dim+1> > CtrlPoints;
	TArray<double> Params;
	GetOpenFormPointsAndParams(CtrlPoints, Params);
	TArray<TArray<TVectorX<Dim+1> > > SplitPosArray;
	TArray<TArray<double> > SplitParamArray;
	DeBoor(T, CtrlPoints, Params, &SplitPosArray, &SplitParamArray);

	if (SplitPosArray.Num() == 0) {
		return;
	}

	TDoubleLinkedList<TClampedBSplineControlPoint<Dim> > LeftList, RightList;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (SplitParamArray[0].Last() < Node->GetValue().Param) {
			RightList.AddTail(Node->GetValue());
		}
		else if (Node->GetValue().Param <= SplitParamArray[0][0]) {
			LeftList.AddTail(Node->GetValue());
		}
		Node = Node->GetNextNode();
	}

	CtrlPointsList.Empty();
	for (const auto& P : LeftList) {
		CtrlPointsList.AddTail(P);
	}
	for (int32 i = 0; i < SplitPosArray[0].Num(); ++i) {
		CtrlPointsList.AddTail(TClampedBSplineControlPoint<Dim>(SplitPosArray[0][i], SplitParamArray[0][i]));
	}
	for (const auto& P : RightList) {
		CtrlPointsList.AddTail(P);
	}
}

//template<int32 Dim, int32 Degree>
//inline void TClampedBSpline<Dim, Degree>::AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom)
//{
//	FPointNode* Node = FindNodeByParam(From, NthPointOfFrom);
//	Node->GetValue().Param = To;
//}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	//double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddTail(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight)));
	AddNewKnotIntervalIfNecessary(Param);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	//double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddHead(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight)));
	AddNewKnotIntervalIfNecessary(Param);
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
			//double InParam = Param ? Param.GetValue() : GetParamRange().Get<0>() - 1.;
			CtrlPointsList.AddHead(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight)));
			AddNewKnotIntervalIfNecessary(Param);
		}
		else {
			while (BeforeNodeToInsertBefore->GetNextNode() != NodeToInsertBefore) {
				BeforeNodeToInsertBefore = BeforeNodeToInsertBefore->GetNextNode();
			}
			//double InParam = Param ? Param.GetValue() :
			//	(NodeToInsertBefore->GetValue().Param + BeforeNodeToInsertBefore->GetValue().Param) * 0.5;
			CtrlPointsList.InsertNode(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight)), NodeToInsertBefore);
			AddNewKnotIntervalIfNecessary(Param);
		}
	}
	else {
		//double InParam = Param ? Param.GetValue() : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
		CtrlPointsList.AddTail(TClampedBSplineControlPoint<Dim>(TVecLib<Dim>::Homogeneous(Point, Weight)));
		AddNewKnotIntervalIfNecessary(Param);
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
inline void TClampedBSpline<Dim, Degree>::Reverse()
{
	// Can B-Spline reverse?
	TDoubleLinkedList<TClampedBSplineControlPoint<Dim> > NewList;
	for (const auto& Point : CtrlPointsList) {
		NewList.AddHead(Point);
	}
	CtrlPointsList.Empty();
	for (const auto& Point : NewList) {
		CtrlPointsList.AddTail(Point);
	}
	if (KnotIntervals.Num() > 1) {
		double SumOfEnd = KnotIntervals[0] + KnotIntervals.Last();
		for (int32 i = 0; i < KnotIntervals.Num(); ++i) {
			KnotIntervals[i] = SumOfEnd - KnotIntervals[i];
		}
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
	GetCtrlPoints(CtrlPoints);
	GetClampedKnotIntervals(Params);

	//return TVecLib<Dim+1>::Projection(CoxDeBoor(T, CtrlPoints, Params));
	return TVecLib<Dim+1>::Projection(DeBoor(T, CtrlPoints, Params));
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TClampedBSpline<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 0)) {
		return TVecLib<Dim>::Zero();
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
inline void TClampedBSpline<Dim, Degree>::ToPolynomialForm(TArray<TArray<TVectorX<Dim+1> > >& OutPolyForms) const
{
	TArray<TBezierCurve<Dim, Degree> > Beziers;
	ToBezierString(Beziers);
	OutPolyForms.Empty(Beziers.Num());
	for (int32 i = 0; i < Beziers.Num(); ++i) {
		TArray<TVectorX<Dim+1> >& Poly = OutPolyForms.AddDefaulted_GetRef();
		Poly.SetNum(Degree + 1);
		Beziers[i].ToPolynomialForm(Poly.GetData());
	}
}

template<int32 Dim, int32 Degree>
inline TTuple<double, double> TClampedBSpline<Dim, Degree>::GetParamRange() const
{
	if (KnotIntervals.Num() > 2) {
		return MakeTuple(KnotIntervals[0], KnotIntervals[GetMaxKnotIntervalIndex()]);
	}
	return MakeTuple(0., 0.);
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim+1> TClampedBSpline<Dim, Degree>::DeBoor(
	double T, const TArray<TVectorX<Dim+1>>& CtrlPoints, const TArray<double>& Params, 
	TArray<TArray<TVectorX<Dim+1> > >* SplitPosArray, TArray<TArray<double> >* SplitParamArray) const
{
	//TODO: Fix
	int32 ListNum = CtrlPointsList.Num();
	int32 EndI = CtrlPoints.Num() - 1;
	for (int32 i = 0; i + 1 < CtrlPoints.Num(); ++i) {
		if (Params[i] <= T && T < Params[i + 1]) {
			if (EndI == CtrlPoints.Num() - 1) {
				EndI = i;
				break;
			}
		}
	}
	if (EndI == CtrlPoints.Num() - 1) {
		return CtrlPoints.Last();
	}

	TArray<TVectorX<Dim+1> > D;
	D.Reserve(Degree + 1);
	for (int32 j = 0; j <= Degree; ++j) {
		D.Add(CtrlPoints[j + EndI - 1]);
	}

	TArray<double> Ps; // For other operations
	Ps.Reserve(Degree + 1);
	for (int32 j = 0; j <= Degree; ++j) {
		Ps.Add(Params[j + EndI - 1]);
	}

	if (SplitPosArray) {
		SplitPosArray->Empty(Degree);
	}
	if (SplitParamArray) {
		SplitParamArray->Empty(Degree);
	}
	for (int32 r = 1; r <= Degree; ++r) {
		//double Alpha = 0.;
		for (int32 j = Degree; j >= r; --j) {
			double De = Params[j + 1 + EndI - r] - Params[j + EndI - Degree];
			double Alpha = FMath::IsNearlyZero(De) ? 0. : (T - Params[j + EndI - Degree]) / De;
			D[j] = D[j - 1] * (1. - Alpha) + D[j] * Alpha;
			Ps[j] = Ps[j - 1] * (1. - Alpha) + Ps[j] * Alpha;
		}

		if (SplitPosArray) {
			SplitPosArray->AddDefaulted_GetRef().Reserve(Degree + 1 - r);
			for (int32 i = r; i <= Degree; ++i) {
				SplitPosArray->Last().Add(D[i]);
			}
		}
		if (SplitParamArray) {
			SplitParamArray->AddDefaulted_GetRef().Reserve(Degree + 1 - r);
			for (int32 i = r; i <= Degree; ++i) {
				SplitParamArray->Last().Add(Ps[i]);
			}
		}
	}
	return D[Degree];
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim+1> TClampedBSpline<Dim, Degree>::CoxDeBoor(double T, const TArray<TVectorX<Dim+1>>& CtrlPoints, const TArray<double>& Params) const
{
	//TODO: Fix
	int32 ListNum = CtrlPointsList.Num();
	TArray<double> N;
	N.SetNumZeroed(CtrlPoints.Num() - 1);
	int32 EndI = N.Num();
	for (int32 i = 0; i < N.Num(); ++i) {
		if (Params[i] <= T && T < Params[i + 1]) {
			if (EndI == N.Num()) {
				EndI = i;
			}
			//N[i] += 0.5;
			N[i] = 1.;
		}
		//if (Params[i] < T && T <= Params[i + 1]) {
		//	N[i] += 0.5;
		//}
	}
	for (int32 k = 1; k <= Degree; ++k) {
		for (int32 i = 0; i + k < N.Num(); ++i) {
			double De0 = Params[i + k] - Params[i];
			double De1 = Params[i + k + 1] - Params[i + 1];
			double W0 = FMath::IsNearlyZero(De0) ? 0. : (T - Params[i]) / De0;
			double W1 = FMath::IsNearlyZero(De1) ? 1. : (Params[i + k + 1] - T) / De1;
			N[i] = W0 * N[i] + W1 * N[i + 1];
		}
	}
	TVectorX<Dim+1> Position = TVecLib<Dim+1>::Zero();
	if (EndI < N.Num()) {
		for (int32 i = EndI - Degree; i <= EndI; ++i) {
			TVectorX<Dim+1> P = CtrlPoints[i + Degree - 1];
			Position += P * N[i];
		}
	}
	else {
		Position = TVecLib<Dim+1>::Projection(CtrlPoints.Last());
	}
	return Position;
}
