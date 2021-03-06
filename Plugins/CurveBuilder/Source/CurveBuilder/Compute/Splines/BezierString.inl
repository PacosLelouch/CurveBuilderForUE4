// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "BezierString.h"

#define GetValueRef GetValue().Get

template<int32 Dim>
inline TBezierString3<Dim>::TBezierString3(const TBezierString3<Dim>& InSpline)
{
	Type = ESplineType::BezierString;
	for (const FControlPointTypeRef& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(Pos.Get()));
	}
}

template<int32 Dim>
inline TBezierString3<Dim>::TBezierString3(const TArray<TBezierCurve<Dim, 3>>& InCurves)
{
	Type = ESplineType::BezierString;
	FromCurveArray(InCurves);
}

template<int32 Dim>
inline TBezierString3<Dim>& TBezierString3<Dim>::operator=(const TBezierString3<Dim>& InSpline)
{
	Type = ESplineType::BezierString;
	CtrlPointsList.Empty();
	for (const FControlPointTypeRef& Pos : InSpline.CtrlPointsList) {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(Pos.Get()));
	}
	return *this;
}

template<int32 Dim>
inline void TBezierString3<Dim>::FromCurveArray(const TArray<TBezierCurve<Dim, 3>>& InCurves)
{
	Type = ESplineType::BezierString;
	CtrlPointsList.Empty();
	for (int32 i = 0; i < InCurves.Num(); ++i) {
		TVectorX<Dim+1> PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(InCurves[i].GetPoint(0) * 2. - InCurves[i].GetPoint(1), 1.);
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(
			InCurves[i].GetPointHomogeneous(0),
			PrevCtrlPointPos,
			InCurves[i].GetPointHomogeneous(1),
			static_cast<double>(i)));
		if (i > 0) {
			if (!TVecLib<Dim+1>::IsNearlyZero(InCurves[i].GetPointHomogeneous(0) - InCurves[i - 1].GetPointHomogeneous(3))) {
				Reset();
				return;
			}
			CtrlPointsList.GetTail()->GetValueRef().PrevCtrlPointPos = InCurves[i - 1].GetPointHomogeneous(2);
		}
	}
	if (InCurves.Num() > 0) {
		TVectorX<Dim+1> NextCtrlPointPos = TVecLib<Dim>::Homogeneous(InCurves[InCurves.Num() - 1].GetPoint(3) * 2. - InCurves[InCurves.Num() - 1].GetPoint(2), 1.);
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(
			InCurves[InCurves.Num() - 1].GetPointHomogeneous(3),
			InCurves[InCurves.Num() - 1].GetPointHomogeneous(2),
			NextCtrlPointPos,
			static_cast<double>(InCurves.Num())));
	}
}

template<int32 Dim>
inline void TBezierString3<Dim>::Reset(
	const TArray<TVectorX<Dim+1>>& InPos, 
	const TArray<TVectorX<Dim+1>>& InPrev, 
	const TArray<TVectorX<Dim+1>>& InNext, 
	const TArray<double>& InParams,
	const TArray<EEndPointContinuity>& InContinuities)
{
	Type = ESplineType::BezierString;
	CtrlPointsList.Empty();
	for (int32 i = 0; i < InPos.Num(); ++i)
	{
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(InPos[i], InPrev[i], InNext[i], InParams[i], InContinuities[i]));
	}
}

template<int32 Dim>
inline typename typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeByParam(double Param, int32 NthNode) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		if (FMath::IsNearlyEqual(Node->GetValueRef().Param, Param)) {
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
		if (Node->GetValueRef().Param > Param) {
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
inline typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeByPosition(const TVectorX<Dim>& Point, int32 NthNode, double ToleranceSqr) const
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

template<int32 Dim>
inline typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::FindNodeByExtentPosition(const TVectorX<Dim>& ExtentPoint, bool bFront, int32 NthNode, double ToleranceSqr) const
{
	int32 Count = 0;
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		TVectorX<Dim> PointFound = bFront ? TVecLib<Dim+1>::Projection(Node->GetValueRef().NextCtrlPointPos) : TVecLib<Dim+1>::Projection(Node->GetValueRef().PrevCtrlPointPos);
		if (TVecLib<Dim>::SizeSquared(PointFound - ExtentPoint) < ToleranceSqr) {
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
inline void TBezierString3<Dim>::GetCtrlPoints(TArray<TVectorX<Dim+1>>& CtrlPoints) const
{
	CtrlPoints.Empty(CtrlPointsList.Num());
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		CtrlPoints.Add(Node->GetValueRef().Pos);
		Node = Node->GetNextNode();
	}
}

template<int32 Dim>
inline void TBezierString3<Dim>::GetCtrlPointsPrev(TArray<TVectorX<Dim+1>>& CtrlPoints) const
{
	CtrlPoints.Empty(CtrlPointsList.Num());
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		CtrlPoints.Add(Node->GetValueRef().PrevCtrlPointPos);
		Node = Node->GetNextNode();
	}
}

template<int32 Dim>
inline void TBezierString3<Dim>::GetCtrlPointsNext(TArray<TVectorX<Dim+1>>& CtrlPoints) const
{
	CtrlPoints.Empty(CtrlPointsList.Num());
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		CtrlPoints.Add(Node->GetValueRef().NextCtrlPointPos);
		Node = Node->GetNextNode();
	}
}

template<int32 Dim>
inline void TBezierString3<Dim>::GetCtrlParams(TArray<double>& CtrlParams) const
{
	CtrlParams.Empty(CtrlPointsList.Num());
	FPointNode* Node = CtrlPointsList.GetHead();
	while (Node) {
		CtrlParams.Add(Node->GetValueRef().Param);
		Node = Node->GetNextNode();
	}
}

template<int32 Dim>
inline bool TBezierString3<Dim>::ToBezierCurves(TArray<TBezierCurve<Dim, 3> >& BezierCurves, TArray<TTuple<double, double> >* ParamRangesPtr) const
{
	FPointNode* Node = CtrlPointsList.GetHead();
	if (!Node) {
		return false;
	}
	BezierCurves.Empty(CtrlPointsList.Num() - 1);
	if (ParamRangesPtr)
	{
		ParamRangesPtr->Empty(CtrlPointsList.Num() - 1);
	}
	FPointNode* NextNode = Node->GetNextNode();
	while (Node && NextNode) {
		const auto& NodeVal = Node->GetValueRef();
		const auto& NextNodeVal = NextNode->GetValueRef();
		TArray<TVectorX<Dim+1> > CtrlPoints{ NodeVal.Pos, NodeVal.NextCtrlPointPos, NextNodeVal.PrevCtrlPointPos, NextNodeVal.Pos };
		BezierCurves.Emplace(CtrlPoints);
		if (ParamRangesPtr)
		{
			ParamRangesPtr->Emplace(MakeTuple(NodeVal.Param, NextNodeVal.Param));
		}
		Node = Node->GetNextNode();
		NextNode = Node->GetNextNode();
	}
	return true;
}

template<int32 Dim>
inline void TBezierString3<Dim>::GetCtrlPointStructs(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, 3>>>& OutControlPointStructs) const
{
	OutControlPointStructs.Empty(CtrlPointsList.Num());
	for (FPointNode* Node = CtrlPointsList.GetHead(); Node; Node = Node->GetNextNode())
	{
		OutControlPointStructs.Add(TWeakPtr<TSplineBaseControlPoint<Dim, 3>>(Node->GetValue()));
	}
}

template<int32 Dim>
inline TWeakPtr<TSplineBaseControlPoint<Dim, 3>> TBezierString3<Dim>::GetLastCtrlPointStruct() const
{
	return TWeakPtr<TSplineBaseControlPoint<Dim, 3>>(CtrlPointsList.GetTail()->GetValue());
}

template<int32 Dim>
inline TWeakPtr<TSplineBaseControlPoint<Dim, 3>> TBezierString3<Dim>::GetFirstCtrlPointStruct() const
{
	return TWeakPtr<TSplineBaseControlPoint<Dim, 3>>(CtrlPointsList.GetHead()->GetValue());
}

template<int32 Dim>
inline void TBezierString3<Dim>::GetSegParams(TArray<double>& OutParameters) const
{
	OutParameters.Empty(CtrlPointsList.Num());
	for (const FControlPointTypeRef& CPRef : CtrlPointsList)
	{
		OutParameters.Add(CPRef.Get().Param);
	}
}

template<int32 Dim>
inline TSharedRef<TSplineBase<Dim, 3>> TBezierString3<Dim>::CreateSameType(int32 EndContinuity) const
{
	TSharedRef<TSplineBase<Dim, 3> > NewSpline = MakeShared<TBezierString3<Dim> >();
	if (EndContinuity >= 0) {
		FPointNode* CurRefNode = CtrlPointsList.GetTail();
		if (CurRefNode) {
			TVectorX<Dim> CurPos = TVecLib<Dim+1>::Projection(CurRefNode->GetValueRef().Pos);
			TVectorX<Dim> CurRefPos = CurPos;
			TBezierString3<Dim>& Beziers = static_cast<TBezierString3<Dim>&>(NewSpline.Get());
			Beziers.AddPointAtLast(CurPos);
			for (int32 i = 0; i < EndContinuity && CurRefNode; i += 2) {
				FPointNode* PrevRefNode = CurRefNode->GetPrevNode();
				if (PrevRefNode) {
					const FControlPointType& PrevRevValue = PrevRefNode->GetValueRef();
					TVectorX<Dim> PrevPos = TVecLib<Dim+1>::Projection(PrevRevValue.Pos);
					TVectorX<Dim> PrevPPos = TVecLib<Dim+1>::Projection(PrevRevValue.PrevCtrlPointPos);
					TVectorX<Dim> PrevNPos = TVecLib<Dim+1>::Projection(PrevRevValue.NextCtrlPointPos);
					Beziers.AddPointAtLast(CurPos + (CurRefPos - PrevPos));

					const FControlPointType& LastNodeValue = Beziers.LastNode()->GetValueRef();
					TVectorX<Dim> NextPos = TVecLib<Dim+1>::Projection(LastNodeValue.Pos);
					Beziers.LastNode()->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(NextPos + (PrevPPos - PrevPos), 1.);
					Beziers.LastNode()->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(NextPos + (PrevNPos - PrevPos), 1.);

					CurRefNode = PrevRefNode;
					const FControlPointType& CurRefValue = CurRefNode->GetValueRef();
					CurPos = TVecLib<Dim+1>::Projection(LastNodeValue.Pos);
					CurRefPos = TVecLib<Dim+1>::Projection(CurRefValue.Pos);
				}
			}
			Beziers.FirstNode()->GetValueRef().PrevCtrlPointPos = CtrlPointsList.GetTail()->GetValueRef().PrevCtrlPointPos;
			Beziers.FirstNode()->GetValueRef().NextCtrlPointPos = CtrlPointsList.GetTail()->GetValueRef().NextCtrlPointPos;
		}
	}
	return NewSpline;
}

template<int32 Dim>
inline TSharedRef<TSplineBase<Dim, 3>> TBezierString3<Dim>::Copy() const
{
	return MakeShared<TBezierString3<Dim> >(*this);
}

template<int32 Dim>
inline void TBezierString3<Dim>::ProcessBeforeCreateSameType(TArray<TWeakPtr<TSplineBaseControlPoint<Dim, 3>>>* NewControlPointStructsPtr)
{
	if (NewControlPointStructsPtr)
	{
		NewControlPointStructsPtr->Empty();
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
		if (Node->GetValueRef().Param <= T) {
			auto Val = Node->GetValueRef();
			Val.Param = static_cast<double>(OutFirst.GetCtrlPointNum());
			OutFirst.AddPointAtLast(Val);
			FPointNode* NextNode = Node->GetNextNode();
			if (NextNode && T < NextNode->GetValueRef().Param) {
				SplitCurveCtrlPoints.Add(Val.Pos);
				SplitCurveCtrlPoints.Add(Val.NextCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValueRef().PrevCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValueRef().Pos);
				SplitT = GetNormalizedParam(Node, NextNode, T);
			}
		}
		else {
			auto Val = Node->GetValueRef();
			Val.Param = static_cast<double>(OutSecond.GetCtrlPointNum() + 1);
			OutSecond.AddPointAtLast(Val);
		}
		Node = Node->GetNextNode();
	}
	if (SplitCurveCtrlPoints.Num() != 4) {
		return;
	}

	TBezierCurve<Dim, 3> NewLeft, NewRight;
	TBezierCurve<Dim, 3> SplitCurve(SplitCurveCtrlPoints);
	TVectorX<Dim+1> SplitPos = SplitCurve.Split(NewLeft, NewRight, SplitT);

	OutFirst.LastNode()->GetValueRef().NextCtrlPointPos = NewLeft.GetPointHomogeneous(1);
	OutSecond.FirstNode()->GetValueRef().PrevCtrlPointPos = NewRight.GetPointHomogeneous(2);

	TBezierString3ControlPoint<Dim> Val(
		SplitPos, 
		NewLeft.GetPointHomogeneous(2),
		NewRight.GetPointHomogeneous(1),
		static_cast<double>(OutFirst.GetCtrlPointNum()));
	OutFirst.AddPointAtLast(Val);
	Val.Param = 0.;
	OutSecond.AddPointAtFirst(Val);
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAtLast(const TBezierString3ControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddTail(MakeShared<FControlPointType>(PointStruct));
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAtFirst(const TBezierString3ControlPoint<Dim>& PointStruct)
{
	CtrlPointsList.AddHead(MakeShared<FControlPointType>(PointStruct));
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
		CtrlPointsList.InsertNode(MakeShared<FControlPointType>(PointStruct), NodeToInsertBefore);
	}
	else {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(PointStruct));
	}
}

template<int32 Dim>
inline typename TBezierString3<Dim>::FPointNode* TBezierString3<Dim>::AddPointWithParamWithoutChangingShape(double T)
{
	TTuple<double, double> ParamRange = GetParamRange();
	if (T >= ParamRange.Get<1>() || T <= ParamRange.Get<0>()) {
		return nullptr;
	}

	FPointNode* Node = CtrlPointsList.GetHead();
	FPointNode* NodeToInsertBefore = nullptr;
	TArray<TVectorX<Dim+1> > SplitCurveCtrlPoints;
	SplitCurveCtrlPoints.Reserve(4);
	while (Node) {
		if (Node->GetValueRef().Param <= T) {
			const auto& Val = Node->GetValueRef();
			//Val.Param = static_cast<double>(OutFirst.GetCtrlPointNum());
			//OutFirst.AddEndPoint(Val);
			FPointNode* NextNode = Node->GetNextNode();
			if (NextNode && T < NextNode->GetValueRef().Param) {
				NodeToInsertBefore = NextNode;
				SplitCurveCtrlPoints.Add(Val.Pos);
				SplitCurveCtrlPoints.Add(Val.NextCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValueRef().PrevCtrlPointPos);
				SplitCurveCtrlPoints.Add(NextNode->GetValueRef().Pos);
				//SplitT = (T - Node->GetValueRef().Param) / (NextNode->GetValueRef().Param - Node->GetValueRef().Param);
				break;
			}
		}
		//else {
		//	auto Val = Node->GetValueRef();
		//	Val.Param = static_cast<double>(OutSecond.GetCtrlPointNum() + 1);
		//	OutSecond.AddEndPoint(Node->GetValueRef());
		//}
		Node = Node->GetNextNode();
	}
	if (SplitCurveCtrlPoints.Num() != 4 || !NodeToInsertBefore) {
		return nullptr;
	}
	FPointNode* NodeToInsertAfter = NodeToInsertBefore->GetPrevNode();

	TBezierCurve<Dim, 3> NewLeft, NewRight;
	TBezierCurve<Dim, 3> SplitCurve(SplitCurveCtrlPoints);
	double TN = GetNormalizedParam(NodeToInsertAfter, NodeToInsertBefore, T);
	TVectorX<Dim+1> SplitPos = SplitCurve.Split(NewLeft, NewRight, TN);

	NodeToInsertAfter->GetValueRef().NextCtrlPointPos = NewLeft.GetPointHomogeneous(1);
	NodeToInsertBefore->GetValueRef().PrevCtrlPointPos = NewRight.GetPointHomogeneous(2);

	TBezierString3ControlPoint<Dim> Val(
		SplitPos,
		NewLeft.GetPointHomogeneous(2),
		NewRight.GetPointHomogeneous(1),
		T);
	CtrlPointsList.InsertNode(MakeShared<FControlPointType>(Val), NodeToInsertBefore);
	return NodeToInsertBefore->GetPrevNode();
}

template<int32 Dim>
inline void TBezierString3<Dim>::AdjustCtrlPointParam(double From, double To, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(From, NthPointOfFrom);
	Node->GetValueRef().Param = To;
	UpdateBezierString(Node);
}

template<int32 Dim>
inline void TBezierString3<Dim>::ChangeCtrlPointContinuous(double From, EEndPointContinuity Continuity, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(From, NthPointOfFrom);
	Node->GetValueRef().Continuity = Continuity;
}

template<int32 Dim>
inline bool TBezierString3<Dim>::AdjustCtrlPointTangent(double From, const TVectorX<Dim>& To, bool bNext, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(From, NthPointOfFrom);
	return AdjustCtrlPointTangent(Node, To, bNext, NthPointOfFrom);
}

template<int32 Dim>
inline bool TBezierString3<Dim>::AdjustCtrlPointTangent(FPointNode* Node, const TVectorX<Dim>& To, bool bNext, int32 NthPointOfFrom)
{
	// TODO?
	if (!Node)
	{
		return false;
	}
	TVectorX<Dim+1>* PosToChangePtr = nullptr;
	TVectorX<Dim+1>* PosToChange2Ptr = nullptr;
	TVectorX<Dim+1>* Pos2ToChangePtr = nullptr;
	TVectorX<Dim+1>* Pos2ToChange2Ptr = nullptr;
	if (bNext) {
		PosToChangePtr = &Node->GetValueRef().NextCtrlPointPos;
		PosToChange2Ptr = &Node->GetValueRef().PrevCtrlPointPos;
		if (Node->GetPrevNode()) {
			Pos2ToChange2Ptr = &Node->GetPrevNode()->GetValueRef().NextCtrlPointPos;
		}
		if (Node->GetNextNode()) {
			Pos2ToChangePtr = &Node->GetNextNode()->GetValueRef().PrevCtrlPointPos;
		}
	}
	else {
		PosToChange2Ptr = &Node->GetValueRef().NextCtrlPointPos;
		PosToChangePtr = &Node->GetValueRef().PrevCtrlPointPos;
		//TODO: Should make the changes slighter?
		if (Node->GetPrevNode()) {
			Pos2ToChangePtr = &Node->GetPrevNode()->GetValueRef().NextCtrlPointPos;
		}
		if (Node->GetNextNode()) {
			Pos2ToChange2Ptr = &Node->GetNextNode()->GetValueRef().PrevCtrlPointPos;
		}
	}
	TVectorX<Dim> Adjust = To - TVecLib<Dim+1>::Projection(*PosToChangePtr);
	*PosToChangePtr = TVecLib<Dim>::Homogeneous(To, 1.);
	TVectorX<Dim> PosProj = TVecLib<Dim+1>::Projection(Node->GetValueRef().Pos);
	EEndPointContinuity Con = Node->GetValueRef().Continuity;
	if (Con > EEndPointContinuity::C0) {
		TVectorX<Dim> TangentFront = To - PosProj;
		//TVectorX<Dim> Adjust2;
		TVectorX<Dim> CurTangentBack = TVecLib<Dim+1>::Projection(*PosToChange2Ptr) - PosProj;
		TVectorX<Dim> NewTangentBack;
		if (Continuity::IsGeometric(Con)) {
			NewTangentBack = TangentFront.GetSafeNormal() * (-TVecLib<Dim>::Size(CurTangentBack));
		}
		else {
			NewTangentBack = -TangentFront;
		}
		TVectorX<Dim> To2 = PosProj + NewTangentBack;
		*PosToChange2Ptr = TVecLib<Dim>::Homogeneous(To2, 1.);

		if ((Con >= EEndPointContinuity::G2 || Con >= EEndPointContinuity::C2) && Pos2ToChangePtr && Pos2ToChange2Ptr) {
			TVectorX<Dim> Pos22Proj = TVecLib<Dim+1>::Projection(*Pos2ToChange2Ptr);

			TVectorX<Dim> Tangent2Back = Pos22Proj + PosProj - To2 * 2.;
			TVectorX<Dim> NewTangent2Front;
			if (Con >= EEndPointContinuity::C2) {
				NewTangent2Front = Tangent2Back;
			}
			else if (Con >= EEndPointContinuity::G2) {
				TVectorX<Dim> Pos21Proj = TVecLib<Dim+1>::Projection(*Pos2ToChangePtr);
				TVectorX<Dim> Tangent2Front = Pos21Proj + PosProj - To * 2.;
				double RatioSqr = TVecLib<Dim>::SizeSquared(TangentFront) / TVecLib<Dim>::SizeSquared(NewTangentBack);
				NewTangent2Front = Tangent2Back * RatioSqr;
					//Tangent2Back.GetSafeNormal() * (TVecLib<Dim>::Size(Tangent2Front));
			}
			*Pos2ToChangePtr = TVecLib<Dim>::Homogeneous(NewTangent2Front - PosProj + To * 2., 1.);
		}
	}
	UpdateBezierString(Node);
	return true;
}

template<int32 Dim>
inline void TBezierString3<Dim>::RemovePoint(double Param, int32 NthPointOfFrom)
{
	FPointNode* Node = FindNodeByParam(Param, NthPointOfFrom);
	CtrlPointsList.RemoveNode(Node);
}

template<int32 Dim>
inline bool TBezierString3<Dim>::AdjustCtrlPointPos(FPointNode* Node, const TVectorX<Dim>& To, int32 NthPointOfFrom)
{
	TVectorX<Dim> From = TVecLib<Dim+1>::Projection(Node->GetValueRef().Pos);
	Node->GetValueRef().Pos = TVecLib<Dim>::Homogeneous(To, 1.);
	EEndPointContinuity Con = Node->GetValueRef().Continuity;
	if (Con > EEndPointContinuity::C0) {
		TVectorX<Dim> AdjustDiff = To - From;
		Node->GetValueRef().PrevCtrlPointPos += TVecLib<Dim>::Homogeneous(AdjustDiff, 0.);
		Node->GetValueRef().NextCtrlPointPos += TVecLib<Dim>::Homogeneous(AdjustDiff, 0.);

		if (CtrlPointsList.Num() > 1) {
			UpdateBezierString(Node);
		}
	}
	return true;
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAtLast(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	double InParam = Param ? Param.Get(0.) : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
	CtrlPointsList.AddTail(MakeShared<FControlPointType>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam));

	//UpdateBezierString(nullptr);

	FPointNode* Tail = CtrlPointsList.GetTail();
	if (CtrlPointsList.Num() > 1) {
		FPointNode* BeforeTail = Tail->GetPrevNode();
		static constexpr double InvDegreeDbl = 1. / 3.;
		if (CtrlPointsList.Num() > 2) {
			FPointNode* BeforeBeforeTail = BeforeTail->GetPrevNode();
			TVectorX<Dim> BBPosProj = TVecLib<Dim+1>::Projection(BeforeBeforeTail->GetValueRef().Pos);
			TVectorX<Dim> BPosProj = TVecLib<Dim+1>::Projection(BeforeTail->GetValueRef().Pos);
			TVectorX<Dim> PosProj = TVecLib<Dim+1>::Projection(Tail->GetValueRef().Pos);

			TVectorX<Dim> BaseLine = PosProj - BBPosProj;
			double BaseLineFactor = 1. / 6.;
			BeforeTail->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(BPosProj + BaseLine * BaseLineFactor);
			if (BeforeTail->GetValueRef().Continuity > EEndPointContinuity::C0) {
				BeforeTail->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(BPosProj - BaseLine * BaseLineFactor);
			}

			Tail->GetValueRef().PrevCtrlPointPos = (BeforeTail->GetValueRef().NextCtrlPointPos + Tail->GetValueRef().Pos) * 0.5; // ''= 0

			TVectorX<Dim> TangentFront = PosProj - TVecLib<Dim+1>::Projection(Tail->GetValueRef().PrevCtrlPointPos);
			Tail->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(PosProj + TangentFront, 1.);
		}
		else {
			TVectorX<Dim> Diff = TVecLib<Dim+1>::Projection(Tail->GetValueRef().Pos)
				- TVecLib<Dim+1>::Projection(BeforeTail->GetValueRef().Pos);
			TVectorX<Dim> TangentFront = Diff * InvDegreeDbl;
			TVectorX<Dim> Pos0 = TVecLib<Dim+1>::Projection(BeforeTail->GetValueRef().Pos);
			TVectorX<Dim> Pos1 = TVecLib<Dim+1>::Projection(Tail->GetValueRef().Pos);

			Tail->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos1 + TangentFront, 1.);
			BeforeTail->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos0 + TangentFront, 1.);
			Tail->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos1 - TangentFront, 1.);
			BeforeTail->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos0 - TangentFront, 1.);
		}
	}

	UpdateBezierString(CtrlPointsList.GetTail());
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAtFirst(const TVectorX<Dim>& Point, TOptional<double> Param, double Weight)
{
	double InParam = Param ? Param.Get(0.) : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<0>() - 1. : 0.);
	CtrlPointsList.AddHead(MakeShared<FControlPointType>(TVecLib<Dim>::Homogeneous(Point, Weight), InParam));

	//UpdateBezierString(nullptr);

	FPointNode* Head = CtrlPointsList.GetHead();
	if (CtrlPointsList.Num() > 1) {
		FPointNode* AfterHead = Head->GetNextNode();
		static constexpr double InvDegreeDbl = 1. / 3.;
		if (CtrlPointsList.Num() > 2) {
			FPointNode* AfterAfterHead = AfterHead->GetNextNode();

			TVectorX<Dim> AAPosProj = TVecLib<Dim+1>::Projection(AfterAfterHead->GetValueRef().Pos);
			TVectorX<Dim> APosProj = TVecLib<Dim+1>::Projection(AfterHead->GetValueRef().Pos);
			TVectorX<Dim> PosProj = TVecLib<Dim+1>::Projection(Head->GetValueRef().Pos);

			TVectorX<Dim> BaseLine = AAPosProj - PosProj;
			double BaseLineFactor = 1. / 6.;
			AfterHead->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(APosProj - BaseLine * BaseLineFactor);
			if (AfterHead->GetValueRef().Continuity > EEndPointContinuity::C0) {
				AfterHead->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(APosProj + BaseLine * BaseLineFactor);
			}

			Head->GetValueRef().NextCtrlPointPos = (AfterHead->GetValueRef().PrevCtrlPointPos + Head->GetValueRef().Pos) * 0.5; // ''= 0

			TVectorX<Dim> TangentFront = TVecLib<Dim+1>::Projection(Head->GetValueRef().NextCtrlPointPos) - PosProj;
			Head->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(PosProj - TangentFront, 1.);
		}
		else {
			TVectorX<Dim> Diff = TVecLib<Dim+1>::Projection(AfterHead->GetValueRef().Pos)
				- TVecLib<Dim+1>::Projection(Head->GetValueRef().Pos);
			TVectorX<Dim> TangentFront = Diff * InvDegreeDbl;
			TVectorX<Dim> Pos0 = TVecLib<Dim+1>::Projection(Head->GetValueRef().Pos);
			TVectorX<Dim> Pos1 = TVecLib<Dim+1>::Projection(AfterHead->GetValueRef().Pos);

			Head->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos0 - TangentFront, 1.);
			AfterHead->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos1 - TangentFront, 1.);
			Head->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos0 + TangentFront, 1.);
			AfterHead->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(Pos1 + TangentFront, 1.);
		}
	}

	UpdateBezierString(CtrlPointsList.GetHead());
}

template<int32 Dim>
inline void TBezierString3<Dim>::AddPointAt(const TVectorX<Dim>& Point, TOptional<double> Param, int32 Index, double Weight)
{
	double InParam = Param ? Param.Get(0.) : (CtrlPointsList.Num() > 0 ? GetParamRange().Get<1>() + 1. : 0.);
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
		CtrlPointsList.InsertNode(MakeShared<FControlPointType>(PointStruct), NodeToInsertBefore);
		UpdateBezierString(nullptr);
		//UpdateBezierString(NodeToInsertBefore->GetPrevNode());
	}
	else {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(PointStruct));
		UpdateBezierString(nullptr);
		//UpdateBezierString(CtrlPointsList.GetTail());
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
inline void TBezierString3<Dim>::RemovePoint(const TSplineBaseControlPoint<Dim, 3>& TargetPointStruct)
{
	for (FPointNode* Node = CtrlPointsList.GetHead(); Node; Node = Node->GetNextNode())
	{
		if (&Node->GetValueRef() == &TargetPointStruct)
		{
			CtrlPointsList.RemoveNode(Node);
			return;
		}
	}
}

template<int32 Dim>
inline bool TBezierString3<Dim>::AdjustCtrlPointPos(TSplineBaseControlPoint<Dim, 3>& PointStructToAdjust, const TVectorX<Dim>& To, int32 TangentFlag, int32 NthPointOfFrom)
{
	FPointNode* NodeToAdjust = nullptr;
	for (FPointNode* Node = CtrlPointsList.GetHead(); Node; Node = Node->GetNextNode())
	{
		if (&Node->GetValueRef() == &PointStructToAdjust)
		{
			NodeToAdjust = Node;
			break;
		}
	}
	if (!NodeToAdjust)
	{
		return false;
	}

	if (TangentFlag > 0)
	{
		return AdjustCtrlPointTangent(NodeToAdjust, To, true, NthPointOfFrom);
	}
	else if (TangentFlag < 0)
	{
		return AdjustCtrlPointTangent(NodeToAdjust, To, false, NthPointOfFrom);
	}

	return AdjustCtrlPointPos(NodeToAdjust, To, NthPointOfFrom);
}

template<int32 Dim>
inline bool TBezierString3<Dim>::AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 TangentFlag, int32 NthPointOfFrom, double ToleranceSqr)
{
	FPointNode* Node = nullptr;
	if (TangentFlag == 0) {
		Node = FindNodeByPosition(From, NthPointOfFrom, ToleranceSqr);
		if (Node) {
			AdjustCtrlPointPos(Node, To, NthPointOfFrom);
		}
	}
	else {
		bool bNext = TangentFlag > 0;
		Node = FindNodeByExtentPosition(From, bNext, NthPointOfFrom, ToleranceSqr);
		if (Node) {
			AdjustCtrlPointTangent(Node, To, bNext, NthPointOfFrom);
		}
	}
	return Node != nullptr;
}

template<int32 Dim>
inline void TBezierString3<Dim>::Reverse()
{
	if (!CtrlPointsList.GetHead() || !CtrlPointsList.GetTail()) {
		return;
	}
	TDoubleLinkedList<FControlPointTypeRef> NewList;
	double SumParam = CtrlPointsList.GetHead()->GetValueRef().Param + CtrlPointsList.GetTail()->GetValueRef().Param;
	for (const auto& Point : CtrlPointsList) {
		NewList.AddHead(Point);
		NewList.GetHead()->GetValueRef().Param = SumParam - NewList.GetHead()->GetValueRef().Param;
		TVectorX<Dim+1> TempPos = NewList.GetHead()->GetValueRef().PrevCtrlPointPos;
		NewList.GetHead()->GetValueRef().PrevCtrlPointPos = NewList.GetHead()->GetValueRef().NextCtrlPointPos;
		NewList.GetHead()->GetValueRef().NextCtrlPointPos = TempPos;
		//std::swap(NewList.GetHead()->GetValueRef().PrevCtrlPointPos, NewList.GetHead()->GetValueRef().NextCtrlPointPos);
	}
	CtrlPointsList.Empty();
	for (const FControlPointTypeRef& Point : NewList) {
		CtrlPointsList.AddTail(MakeShared<FControlPointType>(Point.Get()));
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
		return TVecLib<Dim+1>::Projection(CtrlPointsList.GetHead()->GetValueRef().Pos);
	}
	const auto& ParamRange = GetParamRange();
	if (ParamRange.Get<0>() >= T) {
		TVecLib<Dim+1>::Projection(CtrlPointsList.GetHead()->GetValueRef().Pos);
	}
	else if(T >= ParamRange.Get<1>()) {
		return TVecLib<Dim+1>::Projection(CtrlPointsList.GetTail()->GetValueRef().Pos);
	}

	FPointNode* EndNode = FindNodeGreaterThanParam(T);
	FPointNode* StartNode = EndNode->GetPrevNode() ? EndNode->GetPrevNode() : EndNode;
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
	if (!EndNode) {
		EndNode = CtrlPointsList.GetTail();
	}
	if (!EndNode) {
		return TVecLib<Dim>::Zero();
	}
	FPointNode* StartNode = EndNode->GetPrevNode();
	if (!StartNode) {
		return TVecLib<Dim>::Zero();
	}
	double TN = GetNormalizedParam(StartNode, EndNode, T);
	TBezierCurve<Dim, 3> Curve = MakeBezierCurve(StartNode, EndNode);
	TBezierCurve<Dim, 2> Hodograph;
	Curve.CreateHodograph(Hodograph);
	
	TVectorX<Dim> Tangent = Hodograph.GetPosition(TN);
	return TVecLib<Dim>::IsNearlyZero(Tangent) ? Hodograph.GetTangent(TN) : Tangent;
}

template<int32 Dim>
inline double TBezierString3<Dim>::GetPlanCurvature(double T, int32 PlanIndex) const
{
	FPointNode* EndNode = FindNodeGreaterThanParam(T);
	if (!EndNode) {
		EndNode = CtrlPointsList.GetTail();
	}
	if (!EndNode) {
		return 0.;
	}
	FPointNode* StartNode = EndNode->GetPrevNode();
	if (!StartNode) {
		return 0.;
	}
	double TN = GetNormalizedParam(StartNode, EndNode, T);
	TBezierCurve<Dim, 3> Curve = MakeBezierCurve(StartNode, EndNode);
	TBezierCurve<Dim, 2> Hodograph;
	Curve.CreateHodograph(Hodograph);
	TBezierCurve<Dim, 1> Hodograph2;
	Hodograph.CreateHodograph(Hodograph2);

	return TVecLib<Dim>::PlanCurvature(Hodograph.GetPosition(TN), Hodograph2.GetPosition(TN), PlanIndex);
}

template<int32 Dim>
inline double TBezierString3<Dim>::GetCurvature(double T) const
{
	FPointNode* EndNode = FindNodeGreaterThanParam(T);
	if (!EndNode) {
		EndNode = CtrlPointsList.GetTail();
	}
	if (!EndNode) {
		return 0.;
	}
	FPointNode* StartNode = EndNode->GetPrevNode();
	if (!StartNode) {
		return 0.;
	}
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
		return MakeTuple(CtrlPointsList.GetHead()->GetValueRef().Param, CtrlPointsList.GetTail()->GetValueRef().Param);
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

	F_Box3 InPosBox = F_Box3({ F_Vec3(InPos) }).ExpandBy(sqrt(ToleranceSqr));
	while (Node && Node->GetNextNode()) {
		FPointNode* Next = Node->GetNextNode();

		TBezierCurve<Dim, 3> NewBezier = MakeBezierCurve(Node, Next);
		if (!NewBezier.GetBox().Intersect(InPosBox))
		{
			Node = Next;
			continue;
		}
		double NewParamNormal = -1.;
		if (NewBezier.FindParamByPosition(NewParamNormal, InPos, ToleranceSqr)) {
			double NewParam = Node->GetValueRef().Param * (1. - NewParamNormal) + Next->GetValueRef().Param * NewParamNormal;
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

		Node = Next;
	}

	if (CurParam) {
		OutParam = CurParam.Get(0.);
		return true;
	}
	return false;
}

template<int32 Dim>
inline bool TBezierString3<Dim>::FindParamsByComponentValue(TArray<double>& OutParams, double InValue, int32 InComponentIndex, double ToleranceSqr) const
{
	FPointNode* Node = CtrlPointsList.GetHead();
	if (!Node) {
		return false;
	}
	TOptional<double> CurParam;
	TOptional<double> CurDistSqr;

	//F_Box3 InPosBox = F_Box3({ F_Vec3(InPos) }).ExpandBy(sqrt(ToleranceSqr));
	while (Node && Node->GetNextNode()) {
		FPointNode* Next = Node->GetNextNode();

		TBezierCurve<Dim, 3> NewBezier = MakeBezierCurve(Node, Next);
		F_Box3 BezierBox = NewBezier.GetBox();
		if (BezierBox.Min[InComponentIndex] > InValue || BezierBox.Max[InComponentIndex] < InValue)
		{
			Node = Next;
			continue;
		}
		TArray<double> LocalParams;
		if (NewBezier.FindParamsByComponentValue(LocalParams, InValue, InComponentIndex, ToleranceSqr)) {
			for (double NewParamNormal : LocalParams)
			{
				double NewParam = Node->GetValueRef().Param * (1. - NewParamNormal) + Next->GetValueRef().Param * NewParamNormal;
				//if (CurParam) {
				//	TVectorX<Dim> NewPos = NewBezier.GetPosition(NewParamNormal);
				//	double NewDistSqr = TVecLib<Dim>::SizeSquared(NewPos - InPos);
				//	if (NewDistSqr < CurDistSqr.Get(0.)) {
				//		CurDistSqr = NewDistSqr;
				//		CurParam = NewParam;
				//	}

				//}
				//else {
					double CurValue = NewBezier.GetPosition(NewParamNormal)[InComponentIndex];
					CurDistSqr = FMath::Square(CurValue - InValue);
					OutParams.Add(NewParam);
					CurParam = NewParam;
				//}
			}
		}

		Node = Next;
	}

	if (CurParam) {
		//OutParam = CurParam.Get(0.);
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
	double De = EndNode->GetValueRef().Param - StartNode->GetValueRef().Param;
	return FMath::IsNearlyZero(De) ? 0.5 : (T - StartNode->GetValueRef().Param) / De;
}

template<int32 Dim>
inline TBezierCurve<Dim, 3> TBezierString3<Dim>::MakeBezierCurve(
	const typename TBezierString3<Dim>::FPointNode* StartNode, 
	const typename TBezierString3<Dim>::FPointNode* EndNode) const
{
	TVectorX<Dim+1> Array[] {
		StartNode->GetValueRef().Pos,
		StartNode->GetValueRef().NextCtrlPointPos,
		EndNode->GetValueRef().PrevCtrlPointPos,
		EndNode->GetValueRef().Pos };
	return TBezierCurve<Dim, 3>(Array);
}

template<int32 Dim>
inline void TBezierString3<Dim>::UpdateBezierString(typename TBezierString3<Dim>::FPointNode* NodeToUpdateFirst)
{
	// Check?
	for (FPointNode* Node = CtrlPointsList.GetHead(); Node && Node->GetNextNode(); Node = Node->GetNextNode())
	{
		ensureAlways(Node->GetValueRef().Param <= Node->GetNextNode()->GetValueRef().Param);
	}

	if (!NodeToUpdateFirst) {
		// Interpolate all
		TArray<TVectorX<Dim+1> > EndPoints;
		GetCtrlPoints(EndPoints);
		if (EndPoints.Num() < 2) {
			return;
		}
		TArray<TBezierCurve<Dim, 3> > Beziers;
		TBezierOperationsDegree3<Dim>::InterpolationC2WithBorder2ndDerivative(Beziers, EndPoints);
		FromCurveArray(Beziers);
		return;
	}

	for (FPointNode* PrevNode = NodeToUpdateFirst->GetPrevNode(); PrevNode; PrevNode = PrevNode->GetPrevNode()) {
		if (!AdjustPointByStaticPointReturnShouldSpread(PrevNode, true)) {
			break;
		}
	}

	for (FPointNode* NextNode = NodeToUpdateFirst->GetNextNode(); NextNode; NextNode = NextNode->GetNextNode()) {
		if (!AdjustPointByStaticPointReturnShouldSpread(NextNode, false)) {
			break;
		}
	}

	//static const TMap<EEndPointContinuity, int32> TypeMap{
	//	{EEndPointContinuity::C0, 0},
	//	{EEndPointContinuity::C1, 1},
	//	{EEndPointContinuity::G1, 1},
	//	{EEndPointContinuity::C2, 2},
	//	{EEndPointContinuity::G2, 2},
	//};
	//int32 PointToAdjustEachSide = 2;//TypeMap[NodeToUpdateFirst->GetValueRef().Continuity];
}

template<int32 Dim>
inline bool TBezierString3<Dim>::AdjustPointByStaticPointReturnShouldSpread(TBezierString3<Dim>::FPointNode* Node, bool bFromNext)
{
	EEndPointContinuity Con = Node->GetValueRef().Continuity;
	if (Con == EEndPointContinuity::C0) {
		return false;
	}

	auto GetNextCtrlPointPosOrReverse = [bFromNext](FPointNode* Node) -> TVectorX<Dim> {
		if (bFromNext) {
			return TVecLib<Dim+1>::Projection(Node->GetValueRef().NextCtrlPointPos);
		}
		else {
			return TVecLib<Dim+1>::Projection(Node->GetValueRef().PrevCtrlPointPos);
		}
	};
	auto GetPrevCtrlPointPosOrReverse = [bFromNext](FPointNode* Node) -> TVectorX<Dim> {
		if (bFromNext) {
			return TVecLib<Dim+1>::Projection(Node->GetValueRef().PrevCtrlPointPos);
		}
		else {
			return TVecLib<Dim+1>::Projection(Node->GetValueRef().NextCtrlPointPos);
		}
	};
	auto GetNextNodeOrReverse = [bFromNext](FPointNode* Node)->FPointNode* {
		if (bFromNext) {
			return Node->GetNextNode();
		}
		else {
			return Node->GetPrevNode();
		}
	};
	auto GetPrevNodeOrReverse = [bFromNext](FPointNode* Node)->FPointNode* {
		if (bFromNext) {
			return Node->GetPrevNode();
		}
		else {
			return Node->GetNextNode();
		}
	};
	auto SetPrevCtrlPointPosOrReverse = [bFromNext](FPointNode* Node, const TVectorX<Dim>& Target) {
		if (bFromNext) {
			Node->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(Target, 1.);
		}
		else {
			Node->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(Target, 1.);
		}
	};
	auto SetNextCtrlPointPosOrReverse = [bFromNext](FPointNode* Node, const TVectorX<Dim>& Target) {
		if (bFromNext) {
			Node->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(Target, 1.);
		}
		else {
			Node->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(Target, 1.);
		}
	};

	//if (bFromNext) {
	TVectorX<Dim> NextProj = GetNextCtrlPointPosOrReverse(Node);
	TVectorX<Dim> CurProj = TVecLib<Dim+1>::Projection(Node->GetValueRef().Pos);
	TVectorX<Dim> PrevProj = GetPrevCtrlPointPosOrReverse(Node);
	TVectorX<Dim> TangentFront = NextProj - CurProj, TangentBack = PrevProj - CurProj;
	if (Continuity::IsGeometric(Con)) {
		double Dot = TVecLib<Dim>::Dot(TangentFront, TangentBack);
		if (Con == EEndPointContinuity::G1 &&
			FMath::IsNearlyEqual(Dot * Dot, TVecLib<Dim>::SizeSquared(TangentFront) * TVecLib<Dim>::SizeSquared(TangentBack))) {
			return false;
		} // P' = k * Q'
		TVectorX<Dim> NewTangentBack = TangentFront.GetSafeNormal() * (-TVecLib<Dim>::Size(TangentBack));
		TVectorX<Dim> NewPrevProj = CurProj + NewTangentBack;
		SetPrevCtrlPointPosOrReverse(Node, NewPrevProj);

		FPointNode* NextNode = GetNextNodeOrReverse(Node);
		FPointNode* PrevNode = GetPrevNodeOrReverse(Node);
		if (NextNode && PrevNode && Con == EEndPointContinuity::G2) { // P'' = k^2 * Q'' + j * Q'
			TVectorX<Dim> PPProj = GetNextCtrlPointPosOrReverse(PrevNode);
			TVectorX<Dim> NNProj = GetPrevCtrlPointPosOrReverse(NextNode);
			TVectorX<Dim> Tangent2Front = CurProj - NextProj * 2. + NNProj;
			TVectorX<Dim> Tangent2Back = CurProj - NewPrevProj * 2. + PPProj;
			double RatioSqr = TVecLib<Dim>::SizeSquared(NewTangentBack) / TVecLib<Dim>::SizeSquared(TangentFront);

			TVectorX<Dim> T2Diff = Tangent2Front * RatioSqr - Tangent2Back;
			double DotDiff = TVecLib<Dim>::Dot(T2Diff, TangentFront);
			double CmpDiff = DotDiff * DotDiff - TVecLib<Dim>::SizeSquared(T2Diff) * TVecLib<Dim>::SizeSquared(TangentFront);

			if (FMath::IsNearlyZero(CmpDiff)) {
				return true;
			}

			double Ratio2 = TVecLib<Dim>::Size(T2Diff) / TVecLib<Dim>::Size(TangentFront);

			//if (FMath::IsNearlyEqual(Dot2 * Dot2, TVecLib<Dim>::SizeSquared(Tangent2Front) * TVecLib<Dim>::SizeSquared(Tangent2Back))) {
			//	return true;
			//}

			TVectorX<Dim> NewTangent2Back = Tangent2Front * RatioSqr;// -TangentFront * Ratio2; // TEST?
				//Tangent2Front.GetSafeNormal()* (TVecLib<Dim>::Size(Tangent2Back));
			TVectorX<Dim> NewPPProj = NewTangent2Back - CurProj + NewPrevProj * 2.;
			SetNextCtrlPointPosOrReverse(PrevNode, NewPPProj);

			//TVectorX<Dim> NewT2Diff = Tangent2Front * RatioSqr - NewTangent2Back;
			//double NewDotDiff = TVecLib<Dim>::Dot(NewT2Diff, TangentFront);
			//double NewCmpDiff = NewDotDiff * NewDotDiff - TVecLib<Dim>::SizeSquared(NewT2Diff) * TVecLib<Dim>::SizeSquared(TangentFront);
			//if (!FMath::IsNearlyZero(NewCmpDiff)) {
			//	check(false);
			//}
		}
	}
	else {
		if (Con == EEndPointContinuity::C1 && TVecLib<Dim>::IsNearlyZero(TangentFront + TangentBack)) {
			return false;
		}
		TVectorX<Dim> NewTangentBack = TangentFront * (-1.);
		TVectorX<Dim> NewPrevProj = CurProj + NewTangentBack;
		SetPrevCtrlPointPosOrReverse(Node, NewPrevProj);
		//Node->GetValueRef().PrevCtrlPointPos = TVecLib<Dim>::Homogeneous(NewPrevProj, 1.);

		FPointNode* NextNode = GetNextNodeOrReverse(Node); //Node->GetNextNode();
		FPointNode* PrevNode = GetPrevNodeOrReverse(Node); //Node->GetPrevNode();
		if (NextNode && PrevNode && Con == EEndPointContinuity::C2) {
			TVectorX<Dim> PPProj = GetNextCtrlPointPosOrReverse(PrevNode); //TVecLib<Dim+1>::Projection(PrevNode->GetValueRef().NextCtrlPointPos);
			TVectorX<Dim> NNProj = GetPrevCtrlPointPosOrReverse(NextNode); //TVecLib<Dim+1>::Projection(NextNode->GetValueRef().PrevCtrlPointPos);
			TVectorX<Dim> Tangent2Front = CurProj - NextProj * 2. + NNProj;
			TVectorX<Dim> Tangent2Back = CurProj - NewPrevProj * 2. + PPProj;
			if (TVecLib<Dim>::IsNearlyZero(Tangent2Front - Tangent2Back)) {
				return true;
			}
			TVectorX<Dim> NewTangent2Back = Tangent2Front * (1.);
			TVectorX<Dim> NewPPProj = NewTangent2Back - CurProj + NewPrevProj * 2.;
			SetNextCtrlPointPosOrReverse(PrevNode, NewPPProj);
			//PrevNode->GetValueRef().NextCtrlPointPos = TVecLib<Dim>::Homogeneous(NewPPProj);
		}
	}
	//}
	//else { // FromPrev
	//	FPointNode* PrevNode = Node->GetPrevNode();
	//	//TODO
	//}
	return true;
}

#undef GetValueRef
