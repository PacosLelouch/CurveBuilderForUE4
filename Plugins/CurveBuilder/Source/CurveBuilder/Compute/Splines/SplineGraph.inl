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
		TSharedPtr<FSplineWrapper>& SplineWrapper = SplineWrappers.Add_GetRef(MakeShareable(new FSplineWrapper{ Splines[i] }));
		InternalGraphForward.Add(SplineWrapper, {});
		InternalGraphBackward.Add(SplineWrapper, {});
		SplineToWrapper.Add(Splines[i], TWeakPtr<FSplineWrapper>(SplineWrapper));
	}
	for (int32 i = 0; i < Splines.Num() - 1; ++i) {
		InternalGraphForward[SplineToWrapper[Splines[i]].Pin()].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers[i + 1]), EContactType::Start });
	}
	for (int32 i = 1; i < Splines.Num(); ++i) {
		InternalGraphBackward[SplineToWrapper[Splines[i]].Pin()].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers[i - 1]), EContactType::End });
	}
	if (bClosed) {
		InternalGraphForward[SplineWrappers.Last()].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers[0]), EContactType::Start });
		InternalGraphBackward[SplineWrappers[0]].Add({ TWeakPtr<FSplineWrapper>(SplineWrappers.Last()), EContactType::End });
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
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineWrapper> TSplineGraph<Dim, 3>::GetSplineWrapper(TWeakPtr<FSplineType> SplineWeakPtr)
{
	if (!SplineWeakPtr.IsValid())
	{
		return TWeakPtr<FSplineWrapper>();
	}
	TWeakPtr<FSplineWrapper>* WrapperWeakPtrPtr = SplineToWrapper.Find(SplineWeakPtr.Pin());
	if (!WrapperWeakPtrPtr)
	{
		return TWeakPtr<FSplineWrapper>();
	}
	return *WrapperWeakPtrPtr;
}

template<int32 Dim>
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineType> TSplineGraph<Dim, 3>::AddDefaulted(ESplineType Type)
{
	TSharedPtr<FSplineType> NewSpline;
	switch (Type) {
	case ESplineType::ClampedBSpline:
		NewSpline = MakeShareable(new TSplineTraitByType<ESplineType::ClampedBSpline, Dim, 3>::FSplineType());
		AddSplineToGraph(NewSpline);
		break;
	case ESplineType::BezierString:
		NewSpline = MakeShareable(new TSplineTraitByType<ESplineType::BezierString, Dim, 3>::FSplineType());
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

	VirtualConnectFromPrevEnd(SplineWeakPtr, Next);
	VirtualConnectFromPrevEnd(Prev, SplineWeakPtr);
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
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineType> TSplineGraph<Dim, 3>::CreateSplineBesidesExisted(TWeakPtr<FSplineType> Prev, EContactType Direction, int32 EndContinuity, TArray<TWeakPtr<FControlPointType>>* NewControlPointStructsPtr)
{
	EContactType InvDirection = (Direction == EContactType::Start) ? EContactType::End : EContactType::Start;
	if (Prev.IsValid()) {
		TSharedPtr<FSplineType> PrevSharedPtr = Prev.Pin();
		PrevSharedPtr->ProcessBeforeCreateSameType(NewControlPointStructsPtr);
		TWeakPtr<FSplineWrapper>* PrevWrapperPtr = SplineToWrapper.Find(PrevSharedPtr);
		if (PrevWrapperPtr) {
			TSharedRef<FSplineType> TempSpline = PrevSharedPtr.Get()->Copy();
			auto& TempSplineGet = TempSpline.Get();
			//TempSplineGet.ProcessBeforeCreateSameType(nullptr);
			auto* GraphPtr1 = &InternalGraphForward;
			auto* GraphPtr2 = &InternalGraphBackward;

			if (Direction == EContactType::Start)
			{
				GraphPtr1 = &InternalGraphBackward;
				GraphPtr2 = &InternalGraphForward;
				TempSplineGet.Reverse();
			}

			TSet<TTuple<FGraphNode, int32> > Cluster;
			GetClusterWithoutSelf(Cluster, PrevSharedPtr, Direction);

			TSharedPtr<FSplineWrapper> PrevWrapperSharedPtr = PrevWrapperPtr->Pin();

			TSharedRef<FSplineType> NewSpline = TempSplineGet.CreateSameType(EndContinuity);
			TSharedPtr<FSplineType> NewSplinePtr(NewSpline);
			TSharedPtr<FSplineWrapper> NewSplineWrapper = MakeShareable(new FSplineWrapper{ NewSplinePtr });

			InternalGraphForward.Add(NewSplineWrapper, {});
			InternalGraphBackward.Add(NewSplineWrapper, { { *PrevWrapperPtr, Direction } });
			//GraphPtr1->Add(NewSplineWrapper, {});
			//GraphPtr2->Add(NewSplineWrapper, { { *PrevWrapperPtr, Direction } });
			for (TTuple<FGraphNode, int32>& Tuple : Cluster)
			{
				if ((Tuple.Value & 1) == 0)
				{
					InternalGraphBackward[NewSplineWrapper].Add(Tuple.Key);
					//(*GraphPtr2)[NewSplineWrapper].Add(Tuple.Key);
				}
			}

			TWeakPtr<FSplineWrapper> NewSplineWrapperWeakPtr(NewSplineWrapper);
			auto* PrevValuePtr = GraphPtr1->Find(PrevWrapperSharedPtr);
			(*PrevValuePtr).Add({ NewSplineWrapperWeakPtr, EContactType::Start });
			//(*PrevValuePtr).Add({ NewSplineWrapperWeakPtr, InvDirection }); // Fixed

			SplineToWrapper.Add(NewSplinePtr, NewSplineWrapperWeakPtr);

			return TWeakPtr<FSplineType>(NewSplinePtr);
		}
	}
	return nullptr;
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::VirtualConnectFromPrevEnd(TWeakPtr<FSplineType> Prev, TWeakPtr<FSplineType> Next, EContactType NextContactType)
{
	VirtualConnect(Prev, Next, EContactType::End, NextContactType);
	//if (Prev.IsValid() && Next.IsValid()) 
	//{
	//	TWeakPtr<FSplineWrapper>* PrevWrapperPtr = SplineToWrapper.Find(Prev.Pin());
	//	TWeakPtr<FSplineWrapper>* NextWrapperPtr = SplineToWrapper.Find(Next.Pin());

	//	if (PrevWrapperPtr && NextWrapperPtr) 
	//	{
	//		auto* PrevValuePtr = InternalGraphForward.Find(PrevWrapperPtr->Pin());
	//		auto* NextValuePtr = PrevValuePtr;
	//		if (NextContactType == EContactType::Start) {
	//			NextValuePtr = InternalGraphBackward.Find(NextWrapperPtr->Pin());
	//		}
	//		else {
	//			NextValuePtr = InternalGraphForward.Find(NextWrapperPtr->Pin());
	//		}
	//		if (PrevValuePtr && NextValuePtr) {
	//			(*PrevValuePtr).Add({ *NextWrapperPtr, NextContactType });
	//			(*NextValuePtr).Add({ *PrevWrapperPtr, EContactType::End });
	//		}
	//	}
	//}
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::VirtualConnect(
	TWeakPtr<FSplineType> Prev, TWeakPtr<FSplineType> Next, 
	EContactType PrevContactType, EContactType NextContactType)
{
	if (Prev.IsValid() && Next.IsValid())
	{
		TWeakPtr<FSplineWrapper>* PrevWrapperPtr = SplineToWrapper.Find(Prev.Pin());
		TWeakPtr<FSplineWrapper>* NextWrapperPtr = SplineToWrapper.Find(Next.Pin());

		if (PrevWrapperPtr && NextWrapperPtr)
		{
			TSet<FGraphNode>* PrevValuePtr = nullptr;
			if (PrevContactType == EContactType::Start) {
				PrevValuePtr = InternalGraphBackward.Find(PrevWrapperPtr->Pin());
			}
			else {
				PrevValuePtr = InternalGraphForward.Find(PrevWrapperPtr->Pin());
			}
				
			TSet<FGraphNode>* NextValuePtr = nullptr;
			if (NextContactType == EContactType::Start) {
				NextValuePtr = InternalGraphBackward.Find(NextWrapperPtr->Pin());
			}
			else {
				NextValuePtr = InternalGraphForward.Find(NextWrapperPtr->Pin());
			}

			if (PrevValuePtr && NextValuePtr) {
				(*PrevValuePtr).Add({ *NextWrapperPtr, NextContactType });
				(*NextValuePtr).Add({ *PrevWrapperPtr, PrevContactType });
			}
		}
	}
}

template<int32 Dim>
inline TWeakPtr<typename TSplineGraph<Dim, 3>::FSplineType> TSplineGraph<Dim, 3>::ConnectAndFill(
	TWeakPtr<FSplineType> Source, TWeakPtr<FSplineType> Target, 
	EContactType SourceContactType, EContactType TargetContactType, 
	bool bFillInSource,
	TArray<TWeakPtr<FControlPointType>>* NewSrcControlPointStructsPtr,
	TArray<TWeakPtr<FControlPointType>>* NewTarControlPointStructsPtr)
{
	if (!Source.IsValid() || !Target.IsValid())
	{
		return nullptr;
	}
	auto* SourceSpline = Source.Pin().Get();
	if (SourceSpline->GetCtrlPointNum() < 2)
	{
		return nullptr;
	}

	if (!bFillInSource)
	{
		TWeakPtr<FSplineType> NewSource = this->CreateSplineBesidesExisted(Source, SourceContactType, 1, NewSrcControlPointStructsPtr);
		return ConnectAndFill(NewSource, Target, EContactType::End, TargetContactType, true, nullptr, NewTarControlPointStructsPtr);
	}

	auto* TargetSpline = Target.Pin().Get();
	if (TargetSpline->GetCtrlPointNum() < 2)
	{
		return nullptr;
	}

	TVectorX<Dim> FirstPos, SecondPos;
	TargetSpline->ProcessBeforeCreateSameType(NewTarControlPointStructsPtr);
	switch (TargetSpline->GetType())
	{
	case ESplineType::ClampedBSpline:
	{
		auto* TBSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, Dim, 3>::FSplineType*>(TargetSpline);
		TSplineTraitByType<ESplineType::ClampedBSpline, Dim, 3>::FSplineType::FPointNode* FirstNode = nullptr;
		TSplineTraitByType<ESplineType::ClampedBSpline, Dim, 3>::FSplineType::FPointNode* SecondNode = nullptr;
		if (TargetContactType == EContactType::Start)
		{
			SecondNode = TBSpline->FirstNode();
			FirstNode = SecondNode->GetNextNode();
		}
		else
		{
			SecondNode = TBSpline->LastNode();
			FirstNode = SecondNode->GetPrevNode();
		}
		SecondPos = TVecLib<Dim+1>::Projection(SecondNode->GetValue().Get().Pos);
		FirstPos = SecondPos + (SecondPos - TVecLib<Dim+1>::Projection(FirstNode->GetValue().Get().Pos)) * (TargetSpline->GetCtrlPointNum() < 2 ? 1. / 3. : 1.);
	}
		break;
	case ESplineType::BezierString:
	{
		auto* TBeziers = static_cast<TSplineTraitByType<ESplineType::BezierString, Dim, 3>::FSplineType*>(TargetSpline);
		TSplineTraitByType<ESplineType::BezierString, Dim, 3>::FSplineType::FPointNode* FirstNode = nullptr;
		if (TargetContactType == EContactType::Start)
		{
			FirstNode = TBeziers->FirstNode();
			FirstPos = TVecLib<Dim+1>::Projection(FirstNode->GetValue().Get().PrevCtrlPointPos);
		}
		else
		{
			FirstNode = TBeziers->LastNode();
			FirstPos = TVecLib<Dim+1>::Projection(FirstNode->GetValue().Get().NextCtrlPointPos);
		}
		SecondPos = TVecLib<Dim+1>::Projection(FirstNode->GetValue().Get().Pos);
	}
		break;
	}

	switch (SourceSpline->GetType())
	{
	case ESplineType::ClampedBSpline:
	{
		auto* SBSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, Dim, 3>::FSplineType*>(SourceSpline);
		if (SourceContactType == EContactType::End)
		{
			SBSpline->AddPointAtLast(FirstPos);
			SBSpline->AddPointAtLast(SecondPos);
			if (NewSrcControlPointStructsPtr)
			{
				(*NewSrcControlPointStructsPtr).Add(TWeakPtr<FControlPointType>(SBSpline->LastNode()->GetPrevNode()->GetValue()));
				(*NewSrcControlPointStructsPtr).Add(TWeakPtr<FControlPointType>(SBSpline->LastNode()->GetValue()));
			}
		}
		else
		{
			SBSpline->AddPointAtFirst(FirstPos);
			SBSpline->AddPointAtFirst(SecondPos);
			if (NewSrcControlPointStructsPtr)
			{
				(*NewSrcControlPointStructsPtr).Add(TWeakPtr<FControlPointType>(SBSpline->FirstNode()->GetValue()));
				(*NewSrcControlPointStructsPtr).Add(TWeakPtr<FControlPointType>(SBSpline->FirstNode()->GetNextNode()->GetValue()));
			}
		}
	}
		break;
	case ESplineType::BezierString:
	{
		auto* SBeziers = static_cast<TSplineTraitByType<ESplineType::BezierString, Dim, 3>::FSplineType*>(SourceSpline);
		if (SourceContactType == EContactType::End)
		{
			SBeziers->AddPointAtLast(SecondPos);
			SBeziers->AdjustCtrlPointPos(SBeziers->LastNode()->GetValue().Get(), FirstPos, -1, 0);
			if (NewSrcControlPointStructsPtr)
			{
				(*NewSrcControlPointStructsPtr).Add(TWeakPtr<FControlPointType>(SBeziers->LastNode()->GetValue()));
			}
		}
		else
		{
			SBeziers->AddPointAtFirst(SecondPos);
			SBeziers->AdjustCtrlPointPos(SBeziers->FirstNode()->GetValue().Get(), FirstPos, 1, 0);
			if (NewSrcControlPointStructsPtr)
			{
				(*NewSrcControlPointStructsPtr).Add(TWeakPtr<FControlPointType>(SBeziers->FirstNode()->GetValue()));
			}
		}
	}
		break;
	}

	VirtualConnect(Source, Target, SourceContactType, TargetContactType);
	return Source;
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
			auto PrevWrapperSP = PrevWrapperPtr->Pin();
			auto* PrevFwdValuePtr = InternalGraphForward.Find(PrevWrapperSP);
			auto* PrevBwdValuePtr = InternalGraphBackward.Find(PrevWrapperSP);
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
				if (Node && Node->ContactType == NextContactType) {
					(*PrevFwdValuePtr).Remove(*Node);
				}
				Node = FindGraphNodeBySplinePtrInSet(*PrevBwdValuePtr, Next);
				if (Node && Node->ContactType == NextContactType) {
					(*PrevBwdValuePtr).Remove(*Node);
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

static constexpr double InvDegree = 1. / 3.;

static double GetEndParam(const TTuple<double, double>& ParamRange, EContactType Type) 
{
	return Type == EContactType::Start ? ParamRange.Key : ParamRange.Value;
}

static double GetEndParamDiffOfBSpline(const TArray<double>& Params, EContactType Type) 
{
	return Type == EContactType::Start ? Params[1] - Params[0] : Params[Params.Num() - 1] - Params[Params.Num() - 2];
}

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::AdjustCtrlPointPos(FControlPointType& PointStructToAdjust, const TVectorX<Dim>& To, TWeakPtr<FSplineType> SplinePtrToAdjust, int32 MoveLevel, int32 TangentFlag, int32 NthPointOfFrom)
{
	bool bFindAdjustSpline = false;

	auto AdjustFuncReturnIfSucceed = [this, &PointStructToAdjust, &To, MoveLevel, NthPointOfFrom, TangentFlag](const TSharedPtr<FSplineType>& SplinePtr) -> bool {
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
		if (Spline.AdjustCtrlPointPos(PointStructToAdjust, To, TangentFlag, NthPointOfFrom))
		{
			AdjustAuxiliaryFunc(SplinePtr, ParamRange, InitialPos, InitialTangent, MoveLevel, NthPointOfFrom);
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
inline void TSplineGraph<Dim, 3>::AdjustCtrlPointPos(
	const TVectorX<Dim>& From, const TVectorX<Dim>& To, TWeakPtr<FSplineType> SplinePtrToAdjust, 
	int32 MoveLevel, int32 TangentFlag, int32 NthPointOfFrom, double ToleranceSqr)
{
	bool bFindAdjustSpline = false;

	auto AdjustFuncReturnIfSucceed = [this, &From, &To, MoveLevel, TangentFlag, NthPointOfFrom, ToleranceSqr](const TSharedPtr<FSplineType>& SplinePtr) -> bool {
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
		if (Spline.AdjustCtrlPointPos(From, To, TangentFlag, NthPointOfFrom, ToleranceSqr))
		{
			AdjustAuxiliaryFunc(SplinePtr, ParamRange, InitialPos, InitialTangent, MoveLevel, NthPointOfFrom);
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
				//TSet<FGraphNode> TempNode = *NextValuePtr;
				//*NextValuePtr = *PrevValuePtr;
				//*PrevValuePtr = TempNode;
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
inline bool TSplineGraph<Dim, 3>::HasConnection(
	TWeakPtr<FSplineType> SplinePtr, EContactType Direction,
	TArray<FGraphNode>* ConnectedSplineNodes) const
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
					if (ConnectedSplineNodes)
					{
						ConnectedSplineNodes->Empty(ConnectionSetPtr->Num());
						for (const FGraphNode& Node : *ConnectionSetPtr)
						{
							ConnectedSplineNodes->Add(Node);
						}
					}
					return ConnectionSetPtr->Num() > 0;
				}
			}
			break;
			case EContactType::End:
			{
				auto* ConnectionSetPtr = InternalGraphForward.Find(WrapperSharedPtr);
				if (ConnectionSetPtr)
				{
					if (ConnectedSplineNodes)
					{
						ConnectedSplineNodes->Empty(ConnectionSetPtr->Num());
						for (const FGraphNode& Node : *ConnectionSetPtr)
						{
							ConnectedSplineNodes->Add(Node);
						}
					}
					return ConnectionSetPtr->Num() > 0;
				}
			}
			break;
			}
		}
	}
	if (ConnectedSplineNodes)
	{
		ConnectedSplineNodes->Empty();
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
		BSpline->ToBezierCurves(Beziers);
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

template<int32 Dim>
inline void TSplineGraph<Dim, 3>::AdjustAuxiliaryFunc(
	const TSharedPtr<FSplineType>& SplinePtr, const TTuple<double, double>& ParamRange,
	const TMap<EContactType, TVectorX<Dim> >& InitialPos,
	const TMap<EContactType, TVectorX<Dim> >& InitialTangent,
	int32 MoveLevel, int32 NthPointOfFrom)
{
	FSplineType& Spline = *SplinePtr.Get();
	static const auto GetEndNodeBezierString = [](TBezierString3<Dim>& Spline, EContactType Type) {
		return Type == EContactType::Start ? Spline.FirstNode() : Spline.LastNode();
	};
	static const auto GetEndNodeBSpline = [](TClampedBSpline<Dim, 3>& Spline, EContactType Type) {
		return Type == EContactType::Start ? Spline.FirstNode() : Spline.LastNode();
	};
	static const auto GetSecondNodeBSpline = [](TClampedBSpline<Dim, 3>& Spline, EContactType Type) {
		return Type == EContactType::Start ? Spline.FirstNode()->GetNextNode() : Spline.LastNode()->GetPrevNode();
	};
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
						TBezierString3<Dim>& BezierString = static_cast<TSplineTraitByType<ESplineType::BezierString, Dim, 3>::FSplineType&>(SplineToAdjust);
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
						TClampedBSpline<Dim, 3>& BSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, Dim, 3>::FSplineType&>(SplineToAdjust);
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
}
