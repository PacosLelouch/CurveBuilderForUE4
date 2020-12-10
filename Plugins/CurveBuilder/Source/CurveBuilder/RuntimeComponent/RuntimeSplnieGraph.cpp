// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplineGraph.h"

void URuntimeSplineGraph::GetOwningSplines(TArray<URuntimeCustomSplineBaseComponent*>& Splines)
{
	Splines.Empty(SplineComponentMap.Num());
	for (auto& SpPair : SplineComponentMap)
	{
		Splines.Add(SpPair.Get<1>());
	}
}
