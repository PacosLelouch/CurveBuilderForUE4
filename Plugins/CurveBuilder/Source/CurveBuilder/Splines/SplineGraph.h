// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "BezierString.h"
#include "BSpline.h"

// Only implemented degree 3 version.
template<int32 Dim, int32 Degree = 3>
class TSplineGraph;

template<int32 Dim>
class TSplineGraph<Dim, 3>
{
public:
	using FSplineType = typename TSplineBase<Dim, 3>;

	FORCEINLINE static constexpr int32 SplineDim() { return FSplineType::SplineDim(); }
	FORCEINLINE static constexpr int32 SplineDimHomogeneous() { return FSplineType::SplineDimHomogeneous(); }
	FORCEINLINE static constexpr int32 SplineDegree() { return FSplineType::SplineDegree(); }
	FORCEINLINE static constexpr int32 SplineRank() { return FSplineType::SplineRank(); }

protected:
	// Use a wrapper to keep the pointer after changing the type of the spline.
	struct FSplineWrapper
	{
		TSharedPtr<FSplineType> Spline;
	};

	struct FGraphNode
	{
		TWeakPtr<FSplineWrapper> SplineWrapper;
		EContactType ContactType = Start;

		friend uint32 GetTypeHash(const FGraphNode& Node)
		{
			if (Node.SplineWrapper.IsValid()) {
				return ::PointerHash(Node.SplineWrapper.Pin()->Spline.Get());
			}
			return ::PointerHash(nullptr);
		}

		bool operator==(const FGraphNode& Other) const
		{
			if (!SplineWrapper.IsValid() && !Other.SplineWrapper.IsValid()) {
				return true;
			}
			return SplineWrapper.IsValid() && Other.SplineWrapper.IsValid() && SplineWrapper.Pin()->Spline.Get() == Other.SplineWrapper.Pin()->Spline.Get();
		}
	};

public:
	FORCEINLINE TSplineGraph() {}
	FORCEINLINE virtual ~TSplineGraph() {}

	FORCEINLINE TSplineGraph(const TArray<TSharedPtr<FSplineType> >& Splines, bool bClosed = false);

	virtual void Empty();

	virtual int32 Num() const;

	virtual TWeakPtr<FSplineType> AddDefaulted(ESplineType Type);

	virtual TWeakPtr<FSplineType> AddSplineToGraph(TSharedPtr<FSplineType> Spline, TWeakPtr<FSplineType> Prev = nullptr, TWeakPtr<FSplineType> Next = nullptr);

	virtual TWeakPtr<FSplineType> CreateSplineBesidesExisted(TWeakPtr<FSplineType> Prev, EContactType Direction = EContactType::End, int32 EndContinuity = 1);

	virtual void Connect(TWeakPtr<FSplineType> Previous, TWeakPtr<FSplineType> Next, EContactType NextContactType = EContactType::Start);

	virtual void SplitConnection(TWeakPtr<FSplineType> Previous, TWeakPtr<FSplineType> Next, EContactType NextContactType = EContactType::Start);

	virtual void AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, TWeakPtr<FSplineType> SplinePtrToAdjust = nullptr, int32 MoveLevel = 0, int32 NthPointOfFrom = 0, double ToleranceSqr = 1.);

	// EContactType::End means forward, EContactType::Start means backward.
	virtual void GetClusterWithoutSelf(TSet<TTuple<FGraphNode, int32> >& Cluster, const TSharedPtr<FSplineType>& SplinePtr, EContactType Direction = EContactType::End);

	virtual void GetSplines(TArray<TWeakPtr<FSplineType> >& Splines) const;

	virtual void ReverseSpline(TWeakPtr<FSplineType> SplinePtrToReverse);

	virtual bool HasConnection(TWeakPtr<FSplineType> SplinePtr, EContactType Direction = EContactType::End) const;

	virtual void ChangeSplineType(TWeakPtr<FSplineType>& SplinePtr, ESplineType NewType);

protected:
	TMap<TSharedPtr<FSplineWrapper>, TSet<FGraphNode> > InternalGraphForward;
	TMap<TSharedPtr<FSplineWrapper>, TSet<FGraphNode> > InternalGraphBackward;

	TMap<TSharedPtr<FSplineType>, TWeakPtr<FSplineWrapper> > SplineToWrapper;

	const FGraphNode* FindGraphNodeBySplinePtrInSet(const TSet<FGraphNode>& AdjSet, TWeakPtr<FSplineType> Spline) const;

	void ChangeSplineTypeFromBezierString(TSharedPtr<FSplineWrapper> WrapperSharedPtr, ESplineType NewType);

	void ChangeSplineTypeFromBSpline(TSharedPtr<FSplineWrapper> WrapperSharedPtr, ESplineType NewType);
};

#include "SplineGraph.inl"
