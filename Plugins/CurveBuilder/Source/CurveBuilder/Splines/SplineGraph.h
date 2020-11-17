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

	struct FGraphNode
	{
		TWeakPtr<FSplineType> Spline;
		EContactType ContactType = Start;

		friend uint32 GetTypeHash(const FGraphNode& Node) 
		{
			if (Node.Spline.IsValid()) {
				return ::PointerHash(Node.Spline.Pin().Get());
			}
			return ::PointerHash(nullptr);
		}

		bool operator==(const FGraphNode& Other) const
		{
			if (!Spline.IsValid() && !Other.Spline.IsValid()) {
				return true;
			}
			return Spline.IsValid() && Other.Spline.IsValid() && Spline == Other.Spline;
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

	virtual TWeakPtr<FSplineType> CreateSplineAfterExisted(TWeakPtr<FSplineType> Prev, int32 EndContinuity = 1);

	virtual void Connect(TWeakPtr<FSplineType> Previous, TWeakPtr<FSplineType> Next, EContactType NextContactType = EContactType::Start);

	virtual void SplitConnection(TWeakPtr<FSplineType> Previous, TWeakPtr<FSplineType> Next, EContactType NextContactType = EContactType::Start);

	virtual void AdjustCtrlPointPos(const TVectorX<Dim>& From, const TVectorX<Dim>& To, int32 MoveLevel = 0, int32 NthPointOfFrom = 0, double ToleranceSqr = 1.);

	// EContactType::End means forward, EContactType::Start means backward.
	virtual void GetClusterWithoutSelf(TSet<TTuple<FGraphNode, int32> >& Cluster, const TSharedPtr<FSplineType>& SplinePtr, EContactType Direction = EContactType::End);

	virtual void GetSplines(TArray<TWeakPtr<FSplineType> >& Splines) const;

protected:
	TMap<TSharedPtr<FSplineType>, TSet<FGraphNode> > InternalGraphForward;
	TMap<TSharedPtr<FSplineType>, TSet<FGraphNode> > InternalGraphBackward;

	const FGraphNode* FindGraphNodeBySplinePtrInSet(const TSet<FGraphNode>& AdjSet, TWeakPtr<FSplineType> Spline) const;
};

#include "SplineGraph.inl"
