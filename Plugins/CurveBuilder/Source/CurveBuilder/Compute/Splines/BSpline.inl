// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "BSpline.h"

#define GetValueRef GetValue().Get

template<int32 Dim, int32 Degree>
inline TClampedBSpline<Dim, Degree>::TClampedBSpline(const TClampedBSpline<Dim, Degree>& InSpline)
{
	Type = ESplineType::ClampedBSpline;
	for (const FControlPointTypeRef& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(Pos.Get()));
	}
	for (const auto& P : InSpline.KnotIntervals) {
		KnotIntervals.Add(P);
	}
}

template<int32 Dim, int32 Degree>
inline TClampedBSpline<Dim, Degree>& TClampedBSpline<Dim, Degree>::operator=(const TClampedBSpline<Dim, Degree>& InSpline)
{
	Type = ESplineType::ClampedBSpline;
	CtrlPointsList.Empty();
	KnotIntervals.Empty(InSpline.KnotIntervals.Num());
	for (const FControlPointTypeRef& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(Pos.Get()));
	}
	for (const auto& P : InSpline.KnotIntervals) {
		KnotIntervals.Add(P);
	}
	return *this;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::Reset(const TArray<TVectorX<Dim+1>>& InCtrlPoints, const TArray<double>& InKnotIntervals)
{
	Type = ESplineType::ClampedBSpline;
	CtrlPointsList.Empty();
	KnotIntervals.Empty(InKnotIntervals.Num());
	for (const TVectorX<Dim>& Pos : InCtrlPoints) {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(Pos));
	}
	for (const auto& P : InKnotIntervals) {
		KnotIntervals.Add(P);
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetCtrlPointStructs(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>>& OutControlPointStructs) const
{
	OutControlPointStructs.Empty(CtrlPointsList.Num());
	for (FPointNode* Node = CtrlPointsList.GetHead(); Node; Node = Node->GetNextNode())
	{
		OutControlPointStructs.Add(TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>(Node->GetValue()));
	}
}

template<int32 Dim, int32 Degree>
inline TWeakPtr<TSplineBaseControlPoint<Dim, Degree>> TClampedBSpline<Dim, Degree>::GetLastCtrlPointStruct() const
{
	return TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>(CtrlPointsList.GetTail()->GetValue());
}

template<int32 Dim, int32 Degree>
inline TWeakPtr<TSplineBaseControlPoint<Dim, Degree>> TClampedBSpline<Dim, Degree>::GetFirstCtrlPointStruct() const
{
	return TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>(CtrlPointsList.GetHead()->GetValue());
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetSegParams(TArray<double>& OutParameters) const
{
	GetKnotIntervals(OutParameters);
}

template<int32 Dim, int32 Degree>
inline TSharedRef<TSplineBase<Dim, Degree>> TClampedBSpline<Dim, Degree>::CreateSameType(int32 EndContinuity) const
{
	TSharedRef<TSplineBase<Dim, Degree> > NewSpline = MakeShared<TClampedBSpline<Dim, Degree> >();
	if (EndContinuity >= 0) {
		FPointNode* CurRefNode = CtrlPointsList.GetTail();
		TVectorX<Dim> CurPos = TVecLib<Dim+1>::Projection(CurRefNode->GetValueRef().Pos);
		TVectorX<Dim> CurRefPos = TVecLib<Dim+1>::Projection(CurRefNode->GetValueRef().Pos);
		NewSpline.Get().AddPointAtLast(CurPos);
		for (int32 i = 0; i < EndContinuity; ++i) {
			if (!CurRefNode) {
				break;
			}
			FPointNode* PrevRefNode = CurRefNode->GetPrevNode();
			TVectorX<Dim> PrevRefPos = TVecLib<Dim+1>::Projection(PrevRefNode->GetValueRef().Pos);

			TVectorX<Dim> Diff = CurRefPos - PrevRefPos;
			TVectorX<Dim> NextPos = CurPos + Diff;
			NewSpline.Get().AddPointAtLast(NextPos);

			CurRefNode = PrevRefNode;
			CurPos = NextPos;
			CurRefPos = PrevRefPos;
		}
	}
	return NewSpline;
}

template<int32 Dim, int32 Degree>
inline TSharedRef<TSplineBase<Dim, Degree>> TClampedBSpline<Dim, Degree>::Copy() const
{
	return MakeShared<TClampedBSpline<Dim, Degree> >(*this);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::ProcessBeforeCreateSameType(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, Degree>>>* NewControlPointStructsPtr)
{
	if (CtrlPointsList.Num() == 1) {
		return;
	}

	auto ParamRange = GetParamRange();
	if (NewControlPointStructsPtr)
	{
		NewControlPointStructsPtr->Empty();
	}
	while (CtrlPointsList.Num() <= Degree)
	{
		FPointNode* NewNode = AddPointWithParamWithoutChangingShape(0.5 * (ParamRange.Get<0>() + ParamRange.Get<1>()));
		if (NewControlPointStructsPtr)
		{
			NewControlPointStructsPtr->Add(TWeakPtr<FControlPointType>(NewNode->GetValue()));
		}
	}
}

//template<int32 Dim, int32 Degree>
//inline typename TClampedBSpline<Dim, Degree>::FPointNode* TClampedBSpline<Dim, Degree>::FindNodeByParam(double Param, int32 NthNode) const
//{
//	int32 Count = 0;
//	FPointNode* Node = CtrlPointsList.GetHead();
//	while (Node) {
//		if (FMath::IsNearlyEqual(Node->GetValueRef().Param, Param)) {
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
inline typename TClampedBSpline<Dim, Degree>::FPointNode* TClampedBSpline<Dim, Degree>::FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode, double ToleranceSqr) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (TVecLib<Dim>::SizeSquared(TVecLib<Dim+1>::Projection(Node->GetValueRef().Pos) - Point) < ToleranceSqr) {
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
//		CtrlPoints.Add(Node->GetValueRef().Pos);
//		Params.Add(Node->GetValueRef().Param);
//		//Params.Add(0.);
//	}
//	while (Node) {
//		for (int32 n = 0; n <= Node->GetValueRef().MiddleRepeatNum; ++n) {
//			CtrlPoints.Add(Node->GetValueRef().Pos);
//			Params.Add(Node->GetValueRef().Param);
//			//Params.Add(static_cast<double>(Index));
//		}
//		Node = Node->GetNextNode();
//		//++Index;
//	}
//	if (ListNum > 1) {
//		for (int32 i = 1; i <= RepeatNum; ++i) {
//			CtrlPoints.Add(CtrlPointsList.GetTail()->GetValueRef().Pos);
//			Params.Add(CtrlPointsList.GetTail()->GetValueRef().Param);
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
//		for (int32 n = 0; n <= Node->GetValueRef().MiddleRepeatNum; ++n) {
//			CtrlPoints.Add(Node->GetValueRef().Pos);
//			Params.Add(Node->GetValueRef().Param);
//			//Params.Add(static_cast<double>(Index));
//		}
//		Node = Node->GetNextNode();
//		//++Index;
//	}
//}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetCtrlPoints(TArray<TVectorX<Dim+1>>& CtrlPoints) const
{
	CtrlPoints.Empty(CtrlPointsList.Num());
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		CtrlPoints.Add(Node->GetValueRef().Pos);
		Node = Node->GetNextNode();
	}
}

template<int32 Dim, int32 Degree>
inline bool TClampedBSpline<Dim, Degree>::ToBezierCurves(TArray<TBezierCurve<Dim, Degree> >& BezierCurves, TArray<TTuple<double, double> >* ParamRangesPtr) const
{
	BezierCurves.Empty(KnotIntervals.Num() - 1);

	auto AddBezier = [this, &BezierCurves, ParamRangesPtr](const TClampedBSpline<Dim, Degree>& Spline) {
		TArray<TVectorX<Dim+1> > CtrlPoints;
		Spline.GetCtrlPoints(CtrlPoints);
		if (Spline.GetCtrlPointNum() < 2 || Spline.GetCtrlPointNum() > Degree + 1) { // Invalid
			return;
		}
		//else if (Spline.GetCtrlPointNum() == 2) { // Straight Line
		//	TBezierCurve<Dim, Degree>& NewBezier = Beziers.AddDefaulted_GetRef();
		//	double DegreeDbl = static_cast<double>(Degree);
		//	for (int32 i = 0; i <= Degree; ++i) {
		//		double Alpha = FMath::IsNearlyZero(DegreeDbl) ? 0. : static_cast<double>(i) / DegreeDbl;
		//		NewBezier.SetPointHomogeneous(i, CtrlPoints[0] * Alpha + CtrlPoints[1] * (1. - Alpha));
		//	}
		//	return;
		//}
		else if (Spline.GetCtrlPointNum() <= Degree) { // Incomplete Bezier
			TBezierCurve<Dim, Degree>& NewBezier = BezierCurves.AddDefaulted_GetRef();
			for (int32 i = 0; i <= Degree; ++i) {
				NewBezier.SetPointHomogeneous(i, CtrlPoints[FMath::Min(i, CtrlPoints.Num() - 1)]);
			}
		}
		else { // Complete Bezier
			TBezierCurve<Dim, Degree>& NewBezier = BezierCurves.AddDefaulted_GetRef();
			NewBezier.Reset(CtrlPoints.GetData());
		}
	};

	TClampedBSpline<Dim, Degree> SplitFirst, SplitSecond, ToSplit(*this);
	ToSplit.ProcessBeforeCreateSameType();
	if (ParamRangesPtr)
	{
		ParamRangesPtr->Empty(KnotIntervals.Num() - 1);
	}
	for (int32 i = 1; i + 1 < KnotIntervals.Num(); ++i) {
		ToSplit.Split(SplitFirst, SplitSecond, KnotIntervals[i]);
		AddBezier(SplitFirst);
		ToSplit = SplitSecond;
		if (ParamRangesPtr)
		{
			ParamRangesPtr->Emplace(MakeTuple(KnotIntervals[i - 1], KnotIntervals[i]));
		}
	}
	AddBezier(ToSplit);
	if (ParamRangesPtr)
	{
		ParamRangesPtr->Emplace(MakeTuple(KnotIntervals.Last(1), KnotIntervals.Last(0)));
	}
	return true;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetClampedKnotIntervals(TArray<double>& OutClampedKnotIntervals) const
{
	if (KnotIntervals.Num() == 0) {
		return;
	}
	OutClampedKnotIntervals.Empty(KnotIntervals.Num() + (Degree << 1));
	for (int32 i = 0; i < Degree; ++i) {
		OutClampedKnotIntervals.Add(KnotIntervals[0]);
	}
	for (double KI : KnotIntervals) {
		OutClampedKnotIntervals.Add(KI);
	}
	if (KnotIntervals.Num() > 1) {
		for (int32 i = 0; i < Degree; ++i) {
			OutClampedKnotIntervals.Add(KnotIntervals.Last());
		}
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::GetKnotIntervals(TArray<double>& OutKnotIntervals) const
{
	OutKnotIntervals.Empty(KnotIntervals.Num());
	for (double KI : KnotIntervals) {
		OutKnotIntervals.Add(KI);
	}
}

template<int32 Dim, int32 Degree>
inline int32 TClampedBSpline<Dim, Degree>::GetMaxKnotIntervalIndex() const
{
	// m = n + p + 1, k = n - p + 1, (k + 1) = (n + 1) - p + 1, k = (n + 1) - p 
	int32 MaxIndex = FMath::Max(CtrlPointsList.Num() - Degree, 1);//1 + (CtrlPointsList.Num() - 1) / (Degree + 1);
	return FMath::Max(MaxIndex, KnotIntervals.Num() - 1);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddNewKnotIntervalIfNecessary(TOptional<double> Param)
{
	double InParam = 0.;
	int32 MaxKnotIndexSupportedByCtrlPoints = FMath::Max(CtrlPointsList.Num() - Degree, 1);
	if (KnotIntervals.Num() == 0) {
		InParam = Param ? Param.Get(0.) : 0.;
		KnotIntervals.Add(InParam);
	}
	else if (KnotIntervals.Num() == 1) {
		InParam = Param ? Param.Get(0.) : KnotIntervals.Last() + 1.;
		KnotIntervals.Add(InParam);
	}
	else {
		while (KnotIntervals.Num() < MaxKnotIndexSupportedByCtrlPoints + 1) {
			InParam = Param ? Param.Get(0.) : KnotIntervals.Last() + 1.;
			KnotIntervals.Add(InParam);
		}
	}
	KnotIntervals.Sort();
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::RemoveKnotIntervalIfNecessary()
{
	int32 MaxKnotIndexSupportedByCtrlPoints = FMath::Max(CtrlPointsList.Num() - Degree, 1);
	while (KnotIntervals.Num() > MaxKnotIndexSupportedByCtrlPoints + 1) {
		KnotIntervals.Pop();
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::CreateHodograph(TClampedBSpline<Dim, CLAMP_DEGREE(Degree - 1, 0)>& OutHodograph) const
{
	OutHodograph.Reset();
	FPointNode* CurNode = CtrlPointsList.GetHead();
	if (!CurNode || !CurNode->GetNextNode()) {
		return;
	}
	FPointNode* NextNode = CurNode->GetNextNode();

	TArray<TVectorX<Dim + 1> > CtrlPoints; TArray<double> Params;
	GetCtrlPoints(CtrlPoints);
	GetClampedKnotIntervals(Params);
	constexpr auto DegreeDbl = static_cast<double>(Degree);
	auto Factor = FMath::Min(DegreeDbl, static_cast<double>(CtrlPoints.Num() - 1));
	//constexpr auto DegreeDblInv = Degree == 0 ? 1. : 1 / DegreeDbl;

	TArray<TVectorX<Dim + 1>> HodographPoints;
	HodographPoints.Reserve(CtrlPoints.Num() - 1);
	TArray<double> HodographKnots;
	HodographKnots.Reserve(CtrlPoints.Num() - 1);

	//for (int32 i = 0; i + Degree + 1 < Params.Num(); ++i) {
	for (int32 i = 0; i + 1 < CtrlPoints.Num(); ++i) {
		TVectorX<Dim> DiffPos = TVecLib<Dim + 1>::Projection(CtrlPoints[i + 1]) - TVecLib<Dim + 1>::Projection(CtrlPoints[i]);
		double WN = TVecLib<Dim + 1>::Last(CtrlPoints[i + 1]), WC = TVecLib<Dim + 1>::Last(CtrlPoints[i]);
		double Weight = FMath::IsNearlyZero(WC) ? 1. : WN / WC;
		//double DiffParam = Params[i + 1] - Params[i];
		double DiffParam = Params[i + Degree + 1] - Params[i + 1];
		// H_i = d * \frac{P_{i+1} - P_i}{t_{i+d} - t_i}
		HodographPoints.Add(TVecLib<Dim>::Homogeneous(FMath::IsNearlyZero(DiffParam) ? TVecLib<Dim>::Zero() : DiffPos * Factor / DiffParam, Weight));
		HodographKnots.Add(Params[i + Degree]);
	}

	int32 MaxKnotIndexSupportedByCtrlPointsHodograph = FMath::Max(HodographPoints.Num() - OutHodograph.SplineDegree(), 1);
	while (HodographKnots.Num() > MaxKnotIndexSupportedByCtrlPointsHodograph + 1) {
		HodographKnots.Pop();
	}
	OutHodograph.Reset(HodographPoints, HodographKnots);
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim+1> TClampedBSpline<Dim, Degree>::Split(
	TClampedBSpline<Dim, Degree>& OutFirst, TClampedBSpline<Dim, Degree>& OutSecond, double T,
	TArray<TArray<TVectorX<Dim+1> > >* OutSplitPosArray, int32* OutEndIntervalIndex) const
{
	//TODO: Fix
	OutFirst.Reset();
	OutSecond.Reset();

	TTuple<double, double> ParamRange = GetParamRange();
	if (T >= ParamRange.Get<1>() || T <= ParamRange.Get<0>() || KnotIntervals.Num() <= 2) {
		OutFirst = *this;
		return TVecLib<Dim+1>::Zero();
	}

	TArray<TVectorX<Dim+1> > CtrlPoints;
	TArray<double> Params;

	GetCtrlPoints(CtrlPoints);
	GetClampedKnotIntervals(Params);

	TArray<TArray<TVectorX<Dim+1> > > TempSplitPosArray;
	int32 TempEndIntervalIndex;
	TArray<TArray<TVectorX<Dim+1> > >* SplitPosArrayPtr = OutSplitPosArray ? OutSplitPosArray : &TempSplitPosArray;
	int32* EndIntervalIndexPtr = OutEndIntervalIndex ? OutEndIntervalIndex : &TempEndIntervalIndex;
	TVectorX<Dim+1> ReturnValue = DeBoor(T, CtrlPoints, Params, SplitPosArrayPtr, EndIntervalIndexPtr);

	TArray<FPointNode*> FirstSplits, SecondSplits;
	FirstSplits.Reserve(CtrlPointsList.Num());
	SecondSplits.Reserve(CtrlPointsList.Num());

	int32 k = *EndIntervalIndexPtr;
	for (int32 i = 0; i <= k - Degree; ++i) {
		OutFirst.AddPointAtTailRaw(CtrlPoints[i]);
		OutFirst.AddKnotAtTailRaw(KnotIntervals[i]);
	}
	for (int32 i = 0; i < SplitPosArrayPtr->Num(); ++i) {
		OutFirst.AddPointAtTailRaw((*SplitPosArrayPtr)[i][0]);
	}

	int32 Equal = 1;
	if (!FMath::IsNearlyEqual(T, Params[k])) {
		OutFirst.AddKnotAtTailRaw(T);
		OutSecond.AddKnotAtTailRaw(T);
		Equal = 0;
	}

	for (int32 i = SplitPosArrayPtr->Num() - 1; i >= 0; --i) {
		OutSecond.AddPointAtTailRaw((*SplitPosArrayPtr)[i].Last());
	}
	for (int32 i = k - Equal; i < CtrlPoints.Num(); ++i) {
		OutSecond.AddPointAtTailRaw(CtrlPoints[i]);
		OutSecond.AddKnotAtTailRaw(KnotIntervals.Last(CtrlPoints.Num() - 1 - i));
	}

	return ReturnValue;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtLast(const TClampedBSplineControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddTail(MakeShared<FControlPointType>(PointStruct));
	AddNewKnotIntervalIfNecessary();
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtFirst(const TClampedBSplineControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddHead(MakeShared<FControlPointType>(PointStruct));
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
		CtrlPointsList.InsertNode(MakeShared<FControlPointType>(PointStruct), NodeToInsertBefore);
	}
	else {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(PointStruct));
	}
	AddNewKnotIntervalIfNecessary();
}

template<int32 Dim, int32 Degree>
inline typename TClampedBSpline<Dim, Degree>::FPointNode* TClampedBSpline<Dim, Degree>::AddPointWithParamWithoutChangingShape(double T)
{
	if (CtrlPointsList.Num() <= 1) {
		return nullptr;
	}
	TTuple<double, double> ParamRange = GetParamRange();
	if (T >= ParamRange.Get<1>() || T <= ParamRange.Get<0>()) {
		return nullptr;
	}

	TArray<TVectorX<Dim+1> > CtrlPoints;
	TArray<double> Params;
	GetCtrlPoints(CtrlPoints);
	GetClampedKnotIntervals(Params);

	int32 k = Params.Num() - Degree - 1;
	FPointNode* NodeStart = CtrlPointsList.GetHead();
	for (int32 i = Degree; i < Params.Num() - Degree - 1; ++i) {
		NodeStart = NodeStart->GetNextNode();
		if (Params[i] <= T && T < Params[i + 1]) {
			k = i;
			break;
		}
	}

	FPointNode* NewNode = nullptr;
	for (int32 i = k - Degree + 1; i <= FMath::Min(CtrlPoints.Num() - 1, k); ++i) {
		double De = Params[i + Degree] - Params[i];
		double Alpha = FMath::IsNearlyZero(De) ? 0. : (T - Params[i]) / De;
		TVectorX<Dim+1> NewPos = CtrlPoints[i - 1];
		if (!FMath::IsNearlyZero(Alpha)) {
			NewPos = CtrlPoints[i - 1] * (1. - Alpha) + CtrlPoints[i] * Alpha;
		}

		if (i == k - Degree + 1) {
			CtrlPointsList.InsertNode(MakeShared<FControlPointType>(NewPos), NodeStart);
			NodeStart = NodeStart->GetPrevNode();
			NewNode = NodeStart;
		}
		else {
			NodeStart->GetValueRef().Pos = NewPos;
		}
		NodeStart = NodeStart->GetNextNode();
	}

	int32 MaxKnotIndexSupportedByCtrlPoints = FMath::Max(CtrlPointsList.Num() - Degree, 1);
	if (KnotIntervals.Num() < MaxKnotIndexSupportedByCtrlPoints + 1) {
		KnotIntervals.Insert(T, k - Degree + 1);
	}
	//AddNewKnotIntervalIfNecessary(T);

	return NewNode;

	//TArray<TArray<TVectorX<Dim+1> > > SplitPosArray;
	//TArray<TArray<double> > SplitParamArray;
	//DeBoor(T, CtrlPoints, Params, &SplitPosArray, &SplitParamArray);

	//if (SplitPosArray.Num() == 0) {
	//	return;
	//}

	//TDoubleLinkedList<FControlPointType> LeftList, RightList;
	//FPointNode* Node = CtrlPointsList.GetHead();
	//while (Node) {
	//	if (SplitParamArray[0].Last() < Node->GetValueRef().Param) {
	//		RightList.AddTail(Node->GetValueRef());
	//	}
	//	else if (Node->GetValueRef().Param <= SplitParamArray[0][0]) {
	//		LeftList.AddTail(Node->GetValueRef());
	//	}
	//	Node = Node->GetNextNode();
	//}

	//CtrlPointsList.Empty();
	//for (const auto& P : LeftList) {
	//	CtrlPointsList.AddTail(MakeShared<FControlPointType>(P));
	//}
	//for (int32 i = 0; i < SplitPosArray[0].Num(); ++i) {
	//	CtrlPointsList.AddTail(TClampedBSplineControlPoint<Dim>(SplitPosArray[0][i], SplitParamArray[0][i]));
	//}
	//for (const auto& P : RightList) {
	//	CtrlPointsList.AddTail(MakeShared<FControlPointType>(P));
	//}
}

template<int32 Dim, int32 Degree>
inline bool TClampedBSpline<Dim, Degree>::AdjustCtrlPointPos(FPointNode* Node, const TVectorX<Dim>& To, int32 NthPointOfFrom)
{
	return AdjustCtrlPointPos(Node->GetValueRef(), To, NthPointOfFrom);
}

//template<int32 Dim, int32 Degree>
//inline void TClampedBSpline<Dim, Degree>::AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom)
//{
//	FPointNode* Node = FindNodeByParam(From, NthPointOfFrom);
//	Node->GetValueRef().Param = To;
//}

//template<int32 Dim, int32 Degree>
//inline void TClampedBSpline<Dim, Degree>::RemovePoint(double Param, int32 NthPointOfFrom)
//{
//	FPointNode* Node = FindNodeByParam(Param, NthPointOfFrom);
//	CtrlPointsList.RemoveNode(Node);
//}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	//double InParam = Param ? Param.Get(0.) : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddTail(MakeShared<FControlPointType>(TVecLib<Dim>::Homogeneous(Point, Weight)));
	AddNewKnotIntervalIfNecessary(Param);
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	//double InParam = Param ? Param.Get(0.) : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddHead(MakeShared<FControlPointType>(TVecLib<Dim>::Homogeneous(Point, Weight)));
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
			//double InParam = Param ? Param.Get(0.) : GetParamRange().Get<0>() - 1.;
			CtrlPointsList.AddHead(MakeShared<FControlPointType>(TVecLib<Dim>::Homogeneous(Point, Weight)));
			AddNewKnotIntervalIfNecessary(Param);
		}
		else {
			while (BeforeNodeToInsertBefore->GetNextNode() != NodeToInsertBefore) {
				BeforeNodeToInsertBefore = BeforeNodeToInsertBefore->GetNextNode();
			}
			//double InParam = Param ? Param.Get(0.) :
			//	(NodeToInsertBefore->GetValueRef().Param + BeforeNodeToInsertBefore->GetValueRef().Param) * 0.5;
			CtrlPointsList.InsertNode(MakeShared<FControlPointType>(TVecLib<Dim>::Homogeneous(Point, Weight)), NodeToInsertBefore);
			AddNewKnotIntervalIfNecessary(Param);
		}
	}
	else {
		//double InParam = Param ? Param.Get(0.) : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(TVecLib<Dim>::Homogeneous(Point, Weight)));
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
	if (Node)
	{
		CtrlPointsList.RemoveNode(Node);
		RemoveKnotIntervalIfNecessary();
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::RemovePoint(const TVectorX<Dim>& Point, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByPosition(Point, NthPointOfFrom);
	if (Node)
	{
		CtrlPointsList.RemoveNode(Node);
		RemoveKnotIntervalIfNecessary();
	}
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::RemovePoint(const TSplineBaseControlPoint<Dim, Degree>& TargetPointStruct)
{
	for (FPointNode* Node = CtrlPointsList.GetHead(); Node; Node = Node->GetNextNode())
	{
		if (&Node->GetValueRef() == &TargetPointStruct)
		{
			CtrlPointsList.RemoveNode(Node);
			RemoveKnotIntervalIfNecessary();
			return;
		}
	}
}

template<int32 Dim, int32 Degree>
inline bool TClampedBSpline<Dim, Degree>::AdjustCtrlPointPos(TSplineBaseControlPoint<Dim, Degree>& PointStructToAdjust, const TVectorX<Dim>& To, int32 TangentFlag, int32 NthPointOfFrom)
{
	if (NthPointOfFrom != 0)
	{
		for (FPointNode* Node = CtrlPointsList.GetHead(); Node; Node = Node->GetNextNode())
		{
			if (&Node->GetValueRef() == &PointStructToAdjust)
			{
				return AdjustCtrlPointPos(Node, To, NthPointOfFrom);
			}
		}
		return false;
	}
	static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, Dim, Degree>::FControlPointType&>(PointStructToAdjust).Pos = TVecLib<Dim>::Homogeneous(To, 1.);
	return true;
}

template<int32 Dim, int32 Degree>
inline bool TClampedBSpline<Dim, Degree>::AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 TangentFlag, int32 NthPointOfFrom, double ToleranceSqr)
{
	FPointNode* Node = FindNodeByPosition(From, NthPointOfFrom, ToleranceSqr);
	if (TangentFlag < 0) {
		for (int32 i = 0; i > TangentFlag && Node; --i) {
			Node = Node->GetPrevNode();
		}
	}
	else if (TangentFlag > 0) {
		for (int32 i = 0; i < TangentFlag && Node; ++i) {
			Node = Node->GetNextNode();
		}
	}

	if (!Node) {
		return false;
	}

	AdjustCtrlPointPos(Node, To, NthPointOfFrom);
	return true;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::Reverse()
{
	// Can B-Spline reverse?
	TDoubleLinkedList<TClampedBSplineControlPoint<Dim, Degree> > NewList;
	for (const auto& Point : CtrlPointsList) {
		NewList.AddHead(Point.Get());
	}
	CtrlPointsList.Empty();
	for (const auto& Point : NewList) {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(Point));
	}
	if (KnotIntervals.Num() > 1) {
		double SumOfEnd = KnotIntervals[0] + KnotIntervals.Last();
		for (int32 i = 0; i < KnotIntervals.Num(); ++i) {
			KnotIntervals[KnotIntervals.Num() - 1 - i] = SumOfEnd - KnotIntervals[i];
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
		return TVecLib<Dim + 1>::Projection(CtrlPointsList.GetHead()->GetValueRef().Pos);
	}
	// Number of points are low.
	if (ListNum <= Degree) {
		TClampedBSpline<Dim, Degree> PostProcessSpline(*this);
		PostProcessSpline.ProcessBeforeCreateSameType();
		return PostProcessSpline.GetPosition(T);
	}
	TArray<TVectorX<Dim + 1> > CtrlPoints;
	TArray<double> Params;
	GetCtrlPoints(CtrlPoints);
	GetClampedKnotIntervals(Params);

	//return TVecLib<Dim+1>::Projection(CoxDeBoor(T, CtrlPoints, Params));
	return TVecLib<Dim + 1>::Projection(DeBoor(T, CtrlPoints, Params));
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim> TClampedBSpline<Dim, Degree>::GetTangent(double T) const
{
	if (constexpr(Degree <= 0)) {
		return TVecLib<Dim>::Zero();
	}
	int32 ListNum = CtrlPointsList.Num();
	if (ListNum <= 1) {
		return TVecLib<Dim>::Zero();
	}
	// Number of points are low.
	if (ListNum <= Degree) {
		TClampedBSpline<Dim, Degree> PostProcessSpline(*this);
		PostProcessSpline.ProcessBeforeCreateSameType();
		return PostProcessSpline.GetTangent(T);
	}
	TClampedBSpline<Dim, CLAMP_DEGREE(Degree - 1, 0)> Hodograph;
	CreateHodograph(Hodograph);
	TTuple<double, double> ParamRange = GetParamRange();
	TTuple<double, double> HParamRange = Hodograph.GetParamRange();
	double TH = ConvertRange(T, ParamRange, HParamRange);
	TVectorX<Dim> Tangent = Hodograph.GetPosition(TH);
	return Tangent.IsNearlyZero() ? Hodograph.GetTangent(TH) : Tangent;
}

template<int32 Dim, int32 Degree>
inline double TClampedBSpline<Dim, Degree>::GetPlanCurvature(double T, int32 PlanIndex) const
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
	return TVecLib<Dim>::PlanCurvature(Hodograph.GetPosition(TH), Hodograph2.GetPosition(TH2), PlanIndex);
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
	ToBezierCurves(Beziers);
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
	if (KnotIntervals.Num() > 1) {
		return MakeTuple(KnotIntervals[0], KnotIntervals[GetMaxKnotIntervalIndex()]);
	}
	return MakeTuple(0., 0.);
}

template<int32 Dim, int32 Degree>
inline bool TClampedBSpline<Dim, Degree>::FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr) const
{
	TArray<TBezierCurve<Dim, Degree> > Beziers;
	ToBezierCurves(Beziers);

	TOptional<double> CurParam;
	TOptional<double> CurDistSqr;

	F_Box3 InPosBox = F_Box3({ F_Vec3(InPos) }).ExpandBy(sqrt(ToleranceSqr));
	for (int32 i = 0; i < Beziers.Num(); ++i) {
		const TBezierCurve<Dim, Degree>& NewBezier = Beziers[i];
		if (!NewBezier.GetBox().Intersect(InPosBox))
		{
			continue;
		}
		double NewParamNormal = -1.;
		if (NewBezier.FindParamByPosition(NewParamNormal, InPos, ToleranceSqr)) {
			double NewParam = KnotIntervals[i] * (1. - NewParamNormal) + KnotIntervals[i + 1] * NewParamNormal;
			if (CurParam) {
				TVectorX<Dim> NewPos = NewBezier.GetPosition(NewParamNormal);
				double NewDistSqr = TVecLib<Dim>::SizeSquared(NewPos - InPos);
				if (NewDistSqr < CurDistSqr.Get(0.)) {
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
	}

	if (CurParam) {
		OutParam = CurParam.Get(0.);
		return true;
	}
	return false;
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim+1> TClampedBSpline<Dim, Degree>::DeBoor(
	double T, const TArray<TVectorX<Dim+1>>& CtrlPoints, const TArray<double>& Params, 
	TArray<TArray<TVectorX<Dim+1> > >* SplitPosArray, int32* OutEndIntervalIndex) const
{
	if (OutEndIntervalIndex) {
		(*OutEndIntervalIndex) = -1;
	}
	const auto& ParamRange = GetParamRange();
	if (FMath::IsNearlyEqual(T, ParamRange.Get<0>())) {
		if (OutEndIntervalIndex) {
			(*OutEndIntervalIndex) = Degree;
		}
		return CtrlPoints.Num() > 0 ? CtrlPoints[0] : TVecLib<Dim+1>::Zero();
	}
	else if (FMath::IsNearlyEqual(T, ParamRange.Get<1>())) {
		if (OutEndIntervalIndex) {
			(*OutEndIntervalIndex) = Params.Num() - Degree - 1;
		}
		return CtrlPoints.Last();
	}

	int32 ListNum = CtrlPointsList.Num();
	int32 k = Params.Num() - 1 - Degree;
	static constexpr double ErrorTolerance = SMALL_NUMBER;
	for (int32 i = 0; i + 1 < Params.Num(); ++i) {
		if (FMath::IsNearlyEqual(T, Params[i], ErrorTolerance) || (Params[i] < T && T < Params[i + 1])) {
			k = i;
			break;
		}
	}
	if (OutEndIntervalIndex) {
		(*OutEndIntervalIndex) = k;
	}
	//if (k == Params.Num() - 1) {
	//	return CtrlPoints.Last();
	//}
	int32 H = Degree;
	int32 S = 0;
	if (FMath::IsNearlyEqual(T, Params[k], ErrorTolerance)) {
		for (int32 i = k; i < Params.Num() && FMath::IsNearlyEqual(Params[i], T); ++i) {
			++S;
		}
		H -= S;
	}

	TMap<int32, TVectorX<Dim+1> > D;
	for (int32 i = k - Degree; i <= k - S; ++i) {
		int32 Index = FMath::Min(i, CtrlPoints.Num() - 1); // In case that control point num is LE k.
		D.Add(i, CtrlPoints[Index]);
	}

	if (SplitPosArray) {
		SplitPosArray->Empty(Degree);
	}
	for (int32 r = 1; r <= H; ++r) {
		for (int32 i = k - S; i >= k - Degree + r; --i) {
			double De = Params[i + Degree - r + 1] - Params[i];
			double Alpha = FMath::IsNearlyZero(De) ? 0. : (T - Params[i]) / De;
			D[i] = D[i - 1] * (1. - Alpha) + D[i] * Alpha;
		}

		if (SplitPosArray) {
			SplitPosArray->AddDefaulted_GetRef().Reserve(Degree + 1 - r);
			for (int32 i = k - Degree + r; i <= k - S; ++i) {
				SplitPosArray->Last().Add(D[i]);
			}
		}
	}
	return D[k - S];
}

template<int32 Dim, int32 Degree>
inline TVectorX<Dim+1> TClampedBSpline<Dim, Degree>::CoxDeBoor(double T, const TArray<TVectorX<Dim+1>>& CtrlPoints, const TArray<double>& Params) const
{
	//TODO: Need to fix?
	int32 ListNum = CtrlPointsList.Num();
	TArray<double> N;
	N.SetNumZeroed(Params.Num() - 1);
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
			//TVectorX<Dim+1> P = CtrlPoints[i + Degree - 1];
			TVectorX<Dim+1> P = CtrlPoints[i];
			Position += P * N[i];
		}
	}
	else {
		Position = TVecLib<Dim+1>::Projection(CtrlPoints.Last());
	}
	return Position;
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddPointAtTailRaw(const TVectorX<Dim+1>& CtrlPoint)
{
	CtrlPointsList.AddTail(MakeShared<FControlPointType>(CtrlPoint));
}

template<int32 Dim, int32 Degree>
inline void TClampedBSpline<Dim, Degree>::AddKnotAtTailRaw(double Param)
{
	KnotIntervals.Add(Param);
}

#undef GetValueRef
