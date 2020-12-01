// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineGraph.h"

template<int32 Dim>
inline TSplineGraph<Dim, 3>::TSplineGraph(const TArray<TSharedPtr<FSplineType> >& Splines, bool bClosed)
{
	TArray<TSharedPtr<FSplineWrapper> > SplineWrappers;
	SplineWrappers.Reserve(Splines.Num());
	for (int32 i = 0; i < Splines.Num(); ++i) {
		TSharedPtr<FSplineWrapper>& SplineWrapper = SplineWrappers.Add_GetRef(MakeSheareable(new FSplineWrapper{ Splines[i] }));
		InternalGraphForward.Add(SplineWrapper, {});
		InternalGraphBackward.Add(SplineWrapper, {});
		SplineToWrapper.Add(Splines[i], TWeakPtr<FSplineWrapper>(SplineWrapper));
	}
	for (int32 i = 0; i < Splines.Num() - 1; ++i) {
		InternalGraphForward[Splines[i]].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers[i + 1]), EContactType::Start });
	}
	for (int32 i = 1; i < Splines.Num(); ++i) {
		InternalGraphBackward[Splines[i]].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers[i - 1]), EContactType::End });
	}
	if (bClosed) {
		InternalGraphForward[Splines.Last()].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers[0]), EContactType::Start });
		InternalGraphBackward[Splines[0]].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers.Last()), EContactType::End });
	}
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::Empty()
{
	SplineToWrapper.Empty();
	InternalGraphForward.Empty();
	InternalGraphBackward.Empty();
}

template<int32 Dim>
inline int32 TSplineGraph<Dim, 3>::Num() const
{
	return FMath::Max(InternalGraphForward.Num(), InternalGraphBackward.Num());
}

template<int32 Dim>
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineType> TSplineGraph<Dim, 3>::AddDefaulted(ESplineType Type)
{
	TSharedPtr<FSplineType> NewSpline;
	switch (Type) {
	case ESplineType::ClampedBSpline:
		NewSpline = MakeShareable(new TClampedBSpline<Dim, 3>());
		AddSplineToGraph(NewSpline);
		break;
	case ESplineType::BezierString:
		NewSpline = MakeShareable(new TBezierString3<Dim>());
		AddSplineToGraph(NewSpline);
		break;
	}
	return TWeakPtr<FSplineType>(NewSpline);
}

template<int32 Dim>
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineType> TSplineGraph<Dim, 3>::AddSplineToGraph(TSharedPtr<FSplineType> Spline, TWeakPtr<FSplineType> Prev, TWeakPtr<FSplineType> Next)

{
	TSharedPtr<FSplineWrapper> SplineWrapper = MakeShareable(new FSplineWrapper{ Spline });
	InternalGraphForward.FindOrAdd(SplineWrapper, {});
	InternalGraphBackward.FindOrAdd(SplineWrapper, {});
	SplineToWrapper.FindOrAdd(Spline, TWeakPtr<FSplineWrapper>(SplineWrapper));

	TWeakPtr<FSplineType> SplineWeakPtr(Spline);

	Connect(SplineWeakPtr, Next);
	Connect(Prev, SplineWeakPtr);
	//if (Prev.IsValid()) {
	//	auto* PrevValuePtr = InternalGraphForward.Find(Prev.Pin());
	//	if (PrevValuePtr) {
	//		(*PrevValuePtr).Add(SplineWeakPtr);
	//		auto& CurValueRef = InternalGraphBackward[SplineWeakPtr.Pin()];
	//		CurValueRef.Add(Prev);
	//	}
	//}
	//if (Next.IsValid()) {
	//	auto* NextValuePtr = InternalGraphBackward.Find(Next.Pin());
	//	if (NextValuePtr) {
	//		(*NextValuePtr).Add(SplineWeakPtr);
	//		auto& CurValueRef = InternalGraphForward[SplineWeakPtr.Pin()];
	//		CurValueRef.Add(Next);
	//	}
	//}

	return SplineWeakPtr;
}

template<int32 Dim>
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineType> TSplineGraph<Dim, 3>::CreateSplineBesidesExisted(TWeakPtr<FSplineType> Prev, EContactType Direction, int32 EndContinuity)
{
	EContactType InvDirection = (Direction == EContactType::Start) ? EContactType::End : EContactType::Start;
	if (Prev.IsValid()) {
		TSharedPtr<FSplineType> PrevSharedPtr = Prev.Pin();
		PrevSharedPtr->ProcessBeforeCreateSameType();
		TWeakPtr<FSplineWrapper>* PrevWrapperPtr = SplineToWrapper.Find(PrevSharedPtr);
		if (PrevWrapperPtr) {
			TSharedRef<FSplineType> TempSpline = PrevSharedPtr.Get()->Copy();
			auto* GraphPtr1 = &InternalGraphForward;
			auto* GraphPtr2 = &InternalGraphBackward;

			if (Direction == EContactType::Start)
			{
				GraphPtr1 = &InternalGraphBackward;
				GraphPtr2 = &InternalGraphForward;
				TempSpline.Get().Reverse();
			}

			TSet<TTuple<FGraphNode, int32> > Cluster;
			GetClusterWithoutSelf(Cluster, PrevSharedPtr, Direction);

			TSharedPtr<FSplineWrapper> PrevWrapperSharedPtr = PrevWrapperPtr->Pin();

			TSharedRef<FSplineType> NewSpline = TempSpline.Get().CreateSameType(EndContinuity);
			TSharedPtr<FSplineType> NewSplinePtr(NewSpline);
			TSharedPtr<FSplineWrapper> NewSplineWrapper = MakeShareable(new FSplineWrapper{ NewSplinePtr });

			GraphPtr1->Add(NewSplineWrapper, {});
			GraphPtr2->Add(NewSplineWrapper, { { *PrevWrapperPtr, Direction } });
			for (TTuple<FGraphNode, int32>& Tuple : Cluster)
			{
				if ((Tuple.Value & 1) == 0)
				{
					(*GraphPtr2)[NewSplineWrapper].Add(Tuple.Key);
				}
			}

			TWeakPtr<FSplineWrapper> NewSplineWrapperWeakPtr(NewSplineWrapper);
			auto* PrevValuePtr = GraphPtr1->Find(PrevWrapperSharedPtr);
			(*PrevValuePtr).Add({ NewSplineWrapperWeakPtr, InvDirection });

			SplineToWrapper.Add(NewSplinePtr, NewSplineWrapperWeakPtr);

			return TWeakPtr<FSplineType>(NewSplinePtr);
		}
	}
	return nullptr;
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::Connect(TWeakPtr<FSplineType> Prev, TWeakPtr<FSplineType> Next, EContactType NextContactType)
{
	if (Prev.IsValid() && Next.IsValid()) 
	{
		TWeakPtr<FSplineWrapper>* PrevWrapperPtr = SplineToWrapper.Find(Prev.Pin());
		TWeakPtr<FSplineWrapper>* NextWrapperPtr = SplineToWrapper.Find(Next.Pin());

		if (PrevWrapperPtr && NextWrapperPtr) 
		{
			auto* PrevValuePtr = InternalGraphForward.Find(PrevWrapperPtr->Pin());
			auto* NextValuePtr = PrevValuePtr;
			if (NextContactType == EContactType::Start) {
				NextValuePtr = InternalGraphBackward.Find(NextWrapperPtr->Pin());
			}
			else {
				NextValuePtr = InternalGraphForward.Find(NextWrapperPtr->Pin());
			}
			if (PrevValuePtr && NextValuePtr) {
				(*PrevValuePtr).Add({ *NextWrapperPtr, NextContactType });
				(*NextValuePtr).Add({ *PrevWrapperPtr, EContactType::End });
			}
		}
	}
}


template<int32 Dim>
inline void TSplineGraph<Dim, 3>::SplitConnection(TWeakPtr<FSplineType> Prev, TWeakPtr<FSplineType> Next, EContactType NextContactType)
{
	if (Prev.IsValid() && Next.IsValid()) 
	{
		TWeakPtr<FSplineWrapper>* PrevWrapperPtr = SplineToWrapper.Find(Prev.Pin());
		TWeakPtr<FSplineWrapper>* NextWrapperPtr = SplineToWrapper.Find(Next.Pin());

		if (PrevWrapperPtr && NextWrapperPtr)
		{
			auto* PrevFwdValuePtr = InternalGraphForward.Find(PrevWrapperPtr->Pin());
			auto* PrevBwdValuePtr = InternalGraphBackward.Find(PrevWrapperPtr->Pin());
			//auto* NextFwdValuePtr = InternalGraphForward.Find(NextWrapperPtr->Pin());
			//auto* NextBwdValuePtr = InternalGraphBackward.Find(NextWrapperPtr->Pin());
			auto* NextValuePtr = PrevFwdValuePtr;
			if (NextContactType == EContactType::Start) {
				NextValuePtr = InternalGraphBackward.Find(NextWrapperPtr->Pin());
			}
			else {
				NextValuePtr = InternalGraphForward.Find(NextWrapperPtr->Pin());
			}
			if (PrevFwdValuePtr && PrevBwdValuePtr
				//&& NextFwdValuePtr && NextBwdValuePtr
				&& NextValuePtr) {
				const FGraphNode* Node = FindGraphNodeBySplinePtrInSet(*PrevFwdValuePtr, Next);
				if (Node) {
					(*PrevFwdValuePtr).Remove(*Node);
				}
				Node = FindGraphNodeBySplinePtrInSet(*PrevBwdValuePtr, Next);
				if (Node) {
					(*PrevFwdValuePtr).Remove(*Node);
				}
				Node = FindGraphNodeBySplinePtrInSet(*NextValuePtr, Prev);
				if (Node) {
					(*NextValuePtr).Remove(*Node);
				}
			}
		}
	}
}

template<int32 Dim>
inline bool TSplineGraph<Dim, 3>::DeleteSpline(TWeakPtr<FSplineType> Spline)
{
	if (Spline.IsValid())
	{
		TSharedPtr<FSplineType> SplineSharedPtr = Spline.Pin();
		TWeakPtr<FSplineWrapper>* SplineWrapper = SplineToWrapper.Find(SplineSharedPtr);

		if (SplineWrapper)
		{
			UpdateDeleted(*SplineWrapper);
		}
	}
	return false;
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::AdjustCtrlPointPos(
	const TVectorX<Dim>& From, const TVectorX<Dim>& To, TWeakPtr<FSplineType> SplinePtrToAdjust, 
	int32 MoveLevel, int32 NodeIndexOffset, int32 NthPointOfFrom, double ToleranceSqr)
{
	static constexpr double InvDegree = 1. / 3.;

	static const auto GetEndNodeBezierString = [](TBezierString3<Dim>& Spline, EContactType Type) {
		return Type == EContactType::Start ? Spline.FirstNode() : Spline.LastNode();
	};
	static const auto GetEndNodeBSpline = [](TClampedBSpline<Dim, 3>& Spline, EContactType Type) {
		return Type == EContactType::Start ? Spline.FirstNode() : Spline.LastNode();
	};
	static const auto GetSecondNodeBSpline = [](TClampedBSpline<Dim, 3>& Spline, EContactType Type) {
		return Type == EContactType::Start ? Spline.FirstNode()->GetNextNode() : Spline.LastNode()->GetPrevNode();
	};
	static const auto GetEndParam = [](const TTuple<double, double>& ParamRange, EContactType Type) {
		return Type == EContactType::Start ? ParamRange.Key : ParamRange.Value;
	};
	static const auto GetEndParamDiffOfBSpline = [](const TArray<double>& Params, EContactType Type) {
		return Type == EContactType::Start ? Params[1] - Params[0] : Params[Params.Num() - 1] - Params[Params.Num() - 2];
	};
	
	bool bFindAdjustSpline = false;

	auto AdjustFuncReturnIfSucceed = [this, &From, &To, MoveLevel, NodeIndexOffset, NthPointOfFrom, ToleranceSqr](const TSharedPtr<FSplineType>& SplinePtr) -> bool {
		FSplineType& Spline = *SplinePtr.Get();
		const auto& ParamRange = Spline.GetParamRange();
		TMap<EContactType, TVectorX<Dim> > InitialPos
		{
			{ EContactType::Start, Spline.GetPosition(GetEndParam(ParamRange, EContactType::Start)) },
			{ EContactType::End, Spline.GetPosition(GetEndParam(ParamRange, EContactType::End)) },
		};
		TMap<EContactType, TVectorX<Dim> > InitialTangent
		{
			{ EContactType::Start, Spline.GetTangent(GetEndParam(ParamRange, EContactType::Start)) },
			{ EContactType::End, Spline.GetTangent(GetEndParam(ParamRange, EContactType::End)) },
		};
		if (Spline.AdjustCtrlPointPos(From, To, NodeIndexOffset, NthPointOfFrom, ToleranceSqr))
		{
			for (EContactType ContactTypeToAdjust : { EContactType::Start, EContactType::End })
			{
				TVectorX<Dim> NewEndPos = Spline.GetPosition(GetEndParam(ParamRange, ContactTypeToAdjust));
				TVectorX<Dim> NewEndTangent = Spline.GetTangent(GetEndParam(ParamRange, ContactTypeToAdjust));

				if ((!TVecLib<Dim>::IsNearlyZero(InitialPos[ContactTypeToAdjust] - NewEndPos)) ||
					(!TVecLib<Dim>::IsNearlyZero(InitialTangent[ContactTypeToAdjust] - NewEndTangent)))
				{
					TSet<TTuple<FGraphNode, int32> > EndCluster;
					GetClusterWithoutSelf(EndCluster, SplinePtr, ContactTypeToAdjust);

					for (TTuple<FGraphNode, int32>& NodePair : EndCluster)
					{
						FGraphNode& Node = NodePair.Key;
						int32 Distance = NodePair.Value;
						if (Node.SplineWrapper.IsValid() && Node.SplineWrapper.Pin()->Spline != SplinePtr)
						{
							FSplineType& SplineToAdjust = *Node.SplineWrapper.Pin()->Spline.Get();
							const auto& ParamRangeToAdjust = SplineToAdjust.GetParamRange();
							TVectorX<Dim> TangentToAdjust = SplineToAdjust.GetTangent(GetEndParam(ParamRangeToAdjust, Node.ContactType));
							double SizeNewTangent = TVecLib<Dim>::Size(NewEndTangent);
							double SizeTangentToAdjust = TVecLib<Dim>::Size(TangentToAdjust);
							TVectorX<Dim> NewTangentAdjusted = FMath::IsNearlyZero(SizeNewTangent) ?
								TangentToAdjust :
								NewEndTangent * (SizeTangentToAdjust / SizeNewTangent);

							switch (SplineToAdjust.GetType()) {
							case ESplineType::BezierString:
							{
								TBezierString3<Dim>& BezierString = static_cast<TBezierString3<Dim>&>(SplineToAdjust);
								BezierString.AdjustCtrlPointPos(GetEndNodeBezierString(BezierString, Node.ContactType),
									NewEndPos, NthPointOfFrom);
								if (MoveLevel > 0) {
									double Sgn = ((Distance & 1) == 0 ? 1. : -1.) * (ContactTypeToAdjust == EContactType::Start ? 1. : -1.);
									//double bStart = (ContactTypeToAdjust == EContactType::Start ? true : false);
									BezierString.AdjustCtrlPointTangent(GetEndNodeBezierString(BezierString, Node.ContactType),
										NewEndPos + Sgn * NewTangentAdjusted * InvDegree, 
										(Node.ContactType == EContactType::Start) ? true : false, NthPointOfFrom);
								}
							}
							break;
							case ESplineType::ClampedBSpline:
							{
								TClampedBSpline<Dim, 3>& BSpline = static_cast<TClampedBSpline<Dim, 3>&>(SplineToAdjust);
								BSpline.AdjustCtrlPointPos(GetEndNodeBSpline(BSpline, Node.ContactType),
									NewEndPos, NthPointOfFrom);
								TArray<double> Params;
								BSpline.GetKnotIntervals(Params);
								if (MoveLevel > 0 && Params.Num() > 1) {
									double Sgn = ((Distance & 1) == 0 ? 1. : -1.) * (ContactTypeToAdjust == EContactType::Start ? 1. : -1.);
									double InvD = InvDegree * GetEndParamDiffOfBSpline(Params, Node.ContactType);
									TVectorX<Dim> NewSecondPoint = NewEndPos + Sgn * NewTangentAdjusted * InvD;
									BSpline.AdjustCtrlPointPos(GetSecondNodeBSpline(BSpline, Node.ContactType),
										NewSecondPoint, NthPointOfFrom);
								}
							}
							break;
							}
						}
					}
				}
			}
			return true;
		}
		return false;
	};

	if (SplinePtrToAdjust.IsValid()) 
	{
		AdjustFuncReturnIfSucceed(SplinePtrToAdjust.Pin());
	}
	else 
	{

		TArray<TWeakPtr<FSplineType> > Splines;
		GetSplines(Splines);
		for (int32 i = Splines.Num() - 1; i >= 0; --i)
		{
			if (!Splines[i].IsValid())
			{
				continue;
			}
			const TSharedPtr<FSplineType>& SplinePtr = Splines[i].Pin();
			if (SplinePtr)
			{
				if (AdjustFuncReturnIfSucceed(SplinePtr))
				{
					break;
				}
			}
		}
	}
}


template<int32 Dim>
inline void TSplineGraph<Dim, 3>::GetClusterWithoutSelf(TSet<TTuple<FGraphNode, int32> >& Cluster, const TSharedPtr<FSplineType>& SplinePtr, EContactType Direction)
{
	TWeakPtr<FSplineWrapper>* SplineWrapperPtr = SplineToWrapper.Find(SplinePtr);
	if (!SplineWrapperPtr) {
		return;
	}
	TSharedPtr<FSplineWrapper> SplineWrapperSharedPtr = SplineWrapperPtr->Pin();
	TSet<FGraphNode>* JunctionToVisitNow = (Direction == EContactType::Start) ? InternalGraphBackward.Find(SplineWrapperSharedPtr) : InternalGraphForward.Find(SplineWrapperSharedPtr);
	if (!JunctionToVisitNow) {
		return;
	}

	TSet<TSet<FGraphNode>*> VisitedJunctions;
	TSet<FGraphNode*> VisitedNodes;
	TQueue<TTuple<TSet<FGraphNode>*, int32> > JunctionQueue;
	VisitedJunctions.Add(JunctionToVisitNow);
	JunctionQueue.Enqueue(MakeTuple(JunctionToVisitNow, 1));

	Cluster.Empty(InternalGraphForward.Num());
	//int32 CurNum = Cluster.Num();
	//int32 PrevNum = -1;
	
	TTuple<TSet<FGraphNode>*, int32> VisitTuple;

	while (JunctionQueue.Dequeue(VisitTuple)) {
		JunctionToVisitNow = VisitTuple.Key;
		int32 Distance = VisitTuple.Value;
		if (!JunctionToVisitNow) {
			continue;
		}
		for (FGraphNode& Node : (*JunctionToVisitNow)) {
			if (Node.SplineWrapper.IsValid()) {
				TSharedPtr<FSplineWrapper> CurSplnieWrapperPtr = Node.SplineWrapper.Pin();
				if (CurSplnieWrapperPtr->Spline != SplinePtr) {
					TSet<FGraphNode>* JunctionToVisitSoon = Node.ContactType == EContactType::Start ? InternalGraphBackward.Find(CurSplnieWrapperPtr) : InternalGraphForward.Find(CurSplnieWrapperPtr);
					if (VisitedJunctions.Contains(JunctionToVisitSoon)) {
						continue;
					}
					VisitedJunctions.Add(JunctionToVisitSoon);
					JunctionQueue.Enqueue(MakeTuple(JunctionToVisitSoon, Distance + 1));

					if (VisitedNodes.Contains(&Node)) {
						continue;
					}

					Cluster.Add(MakeTuple(Node, Distance));
					VisitedNodes.Add(&Node);
				}
			}
		}
	}
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::GetSplines(TArray<TWeakPtr<FSplineType> >& Splines) const
{
	Splines.Empty(InternalGraphForward.Num());
	for (const auto& Pair : InternalGraphForward) {
		Splines.Add(TWeakPtr<FSplineType>(Pair.Key->Spline));
	}
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::ReverseSpline(TWeakPtr<FSplineType> SplinePtrToReverse)
{
	if (SplinePtrToReverse.IsValid()) 
	{
		TSharedPtr<FSplineType> SplineSharedPtr = SplinePtrToReverse.Pin();
		SplineSharedPtr->Reverse();

		TWeakPtr<FSplineWrapper>* SplineWrapperWeakPtrPtr = SplineToWrapper.Find(SplineSharedPtr);
		if (SplineWrapperWeakPtrPtr)
		{
			TSharedPtr<FSplineWrapper> SplineWrapperSharedPtr = SplineWrapperWeakPtrPtr->Pin();
			auto* NextValuePtr = InternalGraphForward.Find(SplineWrapperSharedPtr);
			auto* PrevValuePtr = InternalGraphBackward.Find(SplineWrapperSharedPtr);
			if (NextValuePtr && PrevValuePtr)
			{
				Swap(*NextValuePtr, *PrevValuePtr);
			}

			for (auto& Pair : InternalGraphForward)
			{
				for (auto& Node : Pair.Value)
				{
					if (Node.SplineWrapper == SplineWrapperSharedPtr) 
					{
						Node.ContactType = (Node.ContactType == EContactType::Start) ? EContactType::End : EContactType::Start;
					}
				}
			}

			for (auto& Pair : InternalGraphBackward)
			{
				for (auto& Node : Pair.Value)
				{
					if (Node.SplineWrapper == SplineWrapperSharedPtr)
					{
						Node.ContactType = (Node.ContactType == EContactType::Start) ? EContactType::End : EContactType::Start;
					}
				}
			}
		}
	}
}

template<int32 Dim>
inline bool TSplineGraph<Dim, 3>::HasConnection(TWeakPtr<FSplineType> SplinePtr, EContactType Direction) const
{
	if (SplinePtr.IsValid()) 
	{
		const TWeakPtr<FSplineWrapper>* WrapperWeakPtrPtr = SplineToWrapper.Find(SplinePtr.Pin());
		if (WrapperWeakPtrPtr)
		{
			TSharedPtr<FSplineWrapper> WrapperSharedPtr = WrapperWeakPtrPtr->Pin();
			switch (Direction)
			{
			case EContactType::Start:
			{
				auto* ConnectionSetPtr = InternalGraphBackward.Find(WrapperSharedPtr);
				if (ConnectionSetPtr)
				{
					return ConnectionSetPtr->Num() > 0;
				}
			}
			break;
			case EContactType::End:
			{
				auto* ConnectionSetPtr = InternalGraphForward.Find(WrapperSharedPtr);
				if (ConnectionSetPtr)
				{
					return ConnectionSetPtr->Num() > 0;
				}
			}
			break;
			}
		}
	}
	return false;
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::ChangeSplineType(TWeakPtr<FSplineType>& SplinePtr, ESplineType NewType)
{
	if (SplinePtr.IsValid())
	{
		TSharedPtr<FSplineType> SplineSharedPtr = SplinePtr.Pin();
		if (NewType != SplineSharedPtr->GetType()) 
		{
			TWeakPtr<FSplineWrapper>* WrapperWeakPtrPtr = SplineToWrapper.Find(SplineSharedPtr);
			if (WrapperWeakPtrPtr)
			{
				TSharedPtr<FSplineWrapper> WrapperSharedPtr = WrapperWeakPtrPtr->Pin();
				switch (SplineSharedPtr->GetType())
				{
				case ESplineType::BezierString:
					ChangeSplineTypeFromBezierString(WrapperSharedPtr, NewType);
					break;
				case ESplineType::ClampedBSpline:
					ChangeSplineTypeFromBSpline(WrapperSharedPtr, NewType);
					break;
				}
				SplinePtr = TWeakPtr<FSplineType>(WrapperSharedPtr->Spline);
			}
		}
	}
}

template<int32 Dim>
inline const typename TSplineGraph<Dim, 3>::FGraphNode* TSplineGraph<Dim, 3>::FindGraphNodeBySplinePtrInSet(const TSet<FGraphNode>& AdjSet, TWeakPtr<FSplineType> Spline) const
{
	if (Spline.IsValid()) {
		TSharedPtr<FSplineType> SplineShared = Spline.Pin();
		for (const FGraphNode& Node : AdjSet) {
			if (Node.SplineWrapper.Pin()->Spline == SplineShared) {
				return &Node;
			}
		}
	}
	return nullptr;
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::ChangeSplineTypeFromBezierString(TSharedPtr<FSplineWrapper> WrapperSharedPtr, ESplineType NewType)
{
	//TODO
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::ChangeSplineTypeFromBSpline(TSharedPtr<FSplineWrapper> WrapperSharedPtr, ESplineType NewType)
{
	TSharedPtr<FSplineType> SplinePtr = WrapperSharedPtr->Spline;
	switch (NewType)
	{
	case ESplineType::BezierString:
	{
		auto* BSpline = static_cast<TClampedBSpline<Dim, 3>*>(SplinePtr.Get());
		TArray<TBezierCurve<Dim, 3> > Beziers;
		BSpline->ToBezierString(Beziers);
		SplineToWrapper.Remove(WrapperSharedPtr->Spline);
		WrapperSharedPtr->Spline = MakeShareable(new TBezierString3<Dim>(Beziers));
		SplineToWrapper.Add(WrapperSharedPtr->Spline, WrapperSharedPtr);
	}
		break;
	}
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::UpdateDeleted(TWeakPtr<FSplineWrapper> SplineWrapperToDelete)
{
	if (!SplineWrapperToDelete.IsValid())
	{
		return;
	}
	TSharedPtr<FSplineWrapper> WrapperSharedPtr = SplineWrapperToDelete.Pin();
	SplineToWrapper.Remove(WrapperSharedPtr->Spline);

	auto* FwdValuePtr = InternalGraphForward.Find(WrapperSharedPtr);
	auto* BwdValuePtr = InternalGraphBackward.Find(WrapperSharedPtr);

	for (auto* ValuePtr :{ InternalGraphForward.Find(WrapperSharedPtr), InternalGraphBackward.Find(WrapperSharedPtr) })
	{
		if (!ValuePtr)
		{
			continue;
		}
		for (const FGraphNode& Node : *ValuePtr)
		{
			if (!Node.SplineWrapper.IsValid())
			{
				continue;
			}
			TSharedPtr<FSplineWrapper> WrapperSharedPtr1 = Node.SplineWrapper.Pin();
			for (auto* ValuePtr1 :{ InternalGraphForward.Find(WrapperSharedPtr1), InternalGraphBackward.Find(WrapperSharedPtr1) })
			{
				if (ValuePtr1)
				{
					const FGraphNode* NodeToRemove = FindGraphNodeBySplinePtrInSet(*ValuePtr1, WrapperSharedPtr->Spline);
					if (NodeToRemove)
					{
						ValuePtr1->Remove(*NodeToRemove);
					}
				}
			}
		}
	}

	InternalGraphForward.Remove(WrapperSharedPtr);
	InternalGraphBackward.Remove(WrapperSharedPtr);
}
