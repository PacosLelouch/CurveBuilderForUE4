// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"

#pragma once

class URuntimeSplinePointBaseComponent;
class URuntimeCustomSplineBaseComponent;
class ARuntimeSplineGraph;

namespace SplineSerializeUtils
{
	bool ShouldLoadArchive(FArchive& Ar);

	bool ShouldSaveArchive(FArchive& Ar);

	void SerializeSplineWrapper(FArchive& Ar, FSpatialSplineGraph3::FSplineWrapper* SplineWrapper, uint8& bValidSerialize);

	void SerializeSpline(FArchive& Ar, FSpatialSplineGraph3::FSplineType* Spline, ESplineType SpType);

	void DistributeSplinePoints(TSet<URuntimeSplinePointBaseComponent*>& PointComponents, FSpatialSplineGraph3::FSplineType* Spline);
}
