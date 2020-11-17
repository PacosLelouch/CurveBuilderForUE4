// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineGraph.h"

template<int32 Dim>
inline TSplineGraph<Dim, 3>::TSplineGraph(const TArray<TSharedPtr<FSplineType> >& Splines, bool bClosed)
{
	for (int32 i = 0; i < Splines.Num(); ++i) {
		InternalGraphForward.Add(Splines[i], {});
		InternalGraphBackward.Add(Splines[i], {});
	}
	for (int32 i = 0; i < Splines.Num() - 1; ++i) {
		InternalGraphForward[Splines[i]].Add({ TWeakPtr<FSplineType>(Splines[i + 1]), EContactType::Start });
	}
	for (int32 i = 1; i < Splines.Num(); ++i) {
		InternalGraphBackward[Splines[i]].Add({ TWeakPtr<FSplineType>(Splines[i - 1]), EContactType::End });
	}
	if (bClosed) {
		InternalGraphForward[Splines.Last()].Add({ TWeakPtr<FSplineType>(Splines[0]), EContactType::Start });
		InternalGraphBackward[Splines[0]].Add({ TWeakPtr<FSplineType>(Splines.Last()), EContactType::End });
	}
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::Empty()
{
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
	InternalGraphForward.FindOrAdd(Spline, {});
	InternalGraphBackward.FindOrAdd(Spline, {});

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
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineType> TSplineGraph<Dim, 3>::CreateSplineAfterExisted(TWeakPtr<FSplineType> Prev, int32 EndContinuity)
{
	if (Prev.IsValid()) {
		auto* PrevValuePtr = InternalGraphForward.Find(Prev.Pin());
		if (PrevValuePtr) {
			TSharedRef<FSplineType> NewSpline = Prev.Pin().Get()->CreateSameType(EndContinuity);
			TSharedPtr<FSplineType> NewSplinePtr(NewSpline);
			TWeakPtr<FSplineType> NewSplineWeakPtr(NewSplinePtr);
			InternalGraphForward.Add(NewSplinePtr, {});
			InternalGraphBackward.Add(NewSplinePtr, { { Prev.Pin(), EContactType::End } });
			(*PrevValuePtr).Add({ NewSplineWeakPtr, EContactType::Start });

			return NewSplineWeakPtr;
		}
	}
	return TWeakPtr<FSplineType>(nullptr);
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::Connect(TWeakPtr<FSplineType> Prev, TWeakPtr<FSplineType> Next, EContactType NextContactType)
{
	if (Prev.IsValid() && Next.IsValid()) {
		auto* PrevValuePtr = InternalGraphForward.Find(Prev.Pin());
		auto* NextValuePtr = PrevValuePtr;
		if (NextContactType == EContactType::Start) {
			NextValuePtr = InternalGraphBackward.Find(Next.Pin());
		}
		else {
			NextValuePtr = InternalGraphForward.Find(Next.Pin());
		}
		if (PrevValuePtr && NextValuePtr) {
			(*PrevValuePtr).Add({ Next, NextContactType });
			(*NextValuePtr).Add({ Prev, EContactType::End });
		}
	}
}


template<int32 Dim>
inline void TSplineGraph<Dim, 3>::SplitConnection(TWeakPtr<FSplineType> Prev, TWeakPtr<FSplineType> Next, EContactType NextContactType)
{
	if (Prev.IsValid() && Next.IsValid()) {
		auto* PrevFwdValuePtr = InternalGraphForward.Find(Prev.Pin());
		auto* PrevBwdValuePtr = InternalGraphBackward.Find(Prev.Pin());
		//auto* NextFwdValuePtr = InternalGraphForward.Find(Next.Pin());
		//auto* NextBwdValuePtr = InternalGraphBackward.Find(Next.Pin());
		auto* NextValuePtr = PrevFwdValuePtr;
		if (NextContactType == EContactType::Start) {
			NextValuePtr = InternalGraphBackward.Find(Next.Pin());
		}
		else {
			NextValuePtr = InternalGraphForward.Find(Next.Pin());
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

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 MoveLevel, int32 NthPointOfFrom, double ToleranceSqr)
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
	TArray<TWeakPtr<FSplineType> > Splines;
	GetSplines(Splines);
	for (int32 i = Splines.Num() - 1; i >= 0; --i)
	{
		if (!Splines[i].IsValid()) {
			continue;
		}
		const TSharedPtr<FSplineType>& SplinePtr = Splines[i].Pin();
		if (Splines[i].Pin())
		{
			FSplineType& Spline = *SplinePtr.Get();
			for (EContactType ContactTypeToAdjust : { EContactType::Start, EContactType::End }) {
				const auto& ParamRange = Spline.GetParamRange();
				TVectorX<Dim> EndPos = Spline.GetPosition(GetEndParam(ParamRange, ContactTypeToAdjust));
				TVectorX<Dim> EndTangent = Spline.GetTangent(GetEndParam(ParamRange, ContactTypeToAdjust));
				if (Spline.AdjustCtrlPointPos(From, To, 0, NthPointOfFrom, ToleranceSqr))
				{
					bFindAdjustSpline = true;

					TVectorX<Dim> NewEndPos = Spline.GetPosition(GetEndParam(ParamRange, ContactTypeToAdjust));
					TVectorX<Dim> NewEndTangent = Spline.GetTangent(GetEndParam(ParamRange, ContactTypeToAdjust));

					if ((!TVecLib<Dim>::IsNearlyZero(EndPos - NewEndPos)) || (!TVecLib<Dim>::IsNearlyZero(EndTangent - NewEndTangent)))
					{
						TSet<TTuple<FGraphNode, int32> > EndCluster;
						GetClusterWithoutSelf(EndCluster, SplinePtr, ContactTypeToAdjust);

						for (TTuple<FGraphNode, int32>& NodePair : EndCluster)
						{
							FGraphNode& Node = NodePair.Key;
							int32 Distance = NodePair.Value;
							if (Node.Spline.IsValid() && Node.Spline.Pin() != SplinePtr)
							{
								FSplineType& SplineToAdjust = *Node.Spline.Pin().Get();
								switch (SplineToAdjust.GetType()) {
								case ESplineType::BezierString:
								{
									TBezierString3<Dim>& BezierString = static_cast<TBezierString3<Dim>&>(SplineToAdjust);
									BezierString.AdjustCtrlPointPos(GetEndNodeBezierString(BezierString, Node.ContactType),
										NewEndPos, NthPointOfFrom);
									if (MoveLevel > 0) {
										TVectorX<Dim> TangentToAdjust = BezierString.GetTangent(ParamRange.Key);
										BezierString.AdjustCtrlPointTangent(GetEndNodeBezierString(BezierString, Node.ContactType),
											NewEndPos + NewEndTangent * InvDegree, true, NthPointOfFrom);
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
										TVectorX<Dim> NewSecondPoint = NewEndPos + ((Distance & 1) == 0 ? 1 : -1) * NewEndTangent * InvDegree * GetEndParamDiffOfBSpline(Params, Node.ContactType);
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
			}
			if (bFindAdjustSpline) {
				break;
			}
		}
	}
}


template<int32 Dim>
inline void TSplineGraph<Dim, 3>::GetClusterWithoutSelf(TSet<TTuple<FGraphNode, int32> >& Cluster, const TSharedPtr<FSplineType>& SplinePtr, EContactType Direction)
{
	TSet<FGraphNode>* JunctionToVisitNow = Direction == EContactType::Start ? InternalGraphBackward.Find(SplinePtr) : InternalGraphForward.Find(SplinePtr);
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
			if (Node.Spline.IsValid() && Node.Spline.Pin() != SplinePtr) {
				TSet<FGraphNode>* JunctionToVisitSoon = Node.ContactType == EContactType::Start ? InternalGraphBackward.Find(Node.Spline.Pin()) : InternalGraphForward.Find(Node.Spline.Pin());
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

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::GetSplines(TArray<TWeakPtr<FSplineType> >& Splines) const
{
	Splines.Empty(InternalGraphForward.Num());
	for (const auto& Pair : InternalGraphForward) {
		Splines.Add(TWeakPtr<FSplineType>(Pair.Key));
	}
}

template<int32 Dim>
inline const typename TSplineGraph<Dim, 3>::FGraphNode* TSplineGraph<Dim, 3>::FindGraphNodeBySplinePtrInSet(const TSet<FGraphNode>& AdjSet, TWeakPtr<FSplineType> Spline) const
{
	for (const FGraphNode& Node : AdjSet) {
		if (Node.Spline == Spline) {
			return &Node;
		}
	}
	return nullptr;
}
