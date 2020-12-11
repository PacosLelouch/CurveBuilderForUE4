// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.h"

void ARuntimeSplineGraph::Destroyed()
{
	SplineGraphProxy.Empty();
	SplineComponentMap.Empty();
	Super::Destroyed();
}

void ARuntimeSplineGraph::VirtualAttachSplineComponent(URuntimeCustomSplineBaseComponent* SplineComponent)
{
	if (IsValid(SplineComponent))
	{
		SplineComponent->ParentGraph = this;
		SplineComponentMap.Add(SplineComponent->SplineBaseWrapperProxy, SplineComponent);
		SplineComponent->UpdateTransformByCtrlPoint();
	}
}

void ARuntimeSplineGraph::GetOwningSplines(TArray<URuntimeCustomSplineBaseComponent*>& Splines)
{
	Splines.Empty(SplineComponentMap.Num());
	for (auto& SpPair : SplineComponentMap)
	{
		Splines.Add(SpPair.Get<1>());
	}
}

URuntimeCustomSplineBaseComponent* ARuntimeSplineGraph::GetSplineComponentBySplineWeakPtr(TWeakPtr<FSpatialSplineGraph3::FSplineType> SplineWeakPtr)
{
	TWeakPtr<FSpatialSplineGraph3::FSplineWrapper> WrapperWeakPtr = SplineGraphProxy.GetSplineWrapper(SplineWeakPtr);
	if (!WrapperWeakPtr.IsValid())
	{
		return nullptr;
	}
	URuntimeCustomSplineBaseComponent** SpCompPtr = SplineComponentMap.Find(WrapperWeakPtr.Pin());
	if (!SpCompPtr)
	{
		return nullptr;
	}
	return *SpCompPtr;
}
