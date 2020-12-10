// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "OffsetExplicitBSpline.h"

template<int32 Degree>
inline void TOffsetExplicit2ClampedBSpline<Degree>::GetKnotsS(TArray<double>& OutKnotsS) const
{
	OutKnotsS = KnotIntervals;
}

template<int32 Degree>
template<int32 DimOri>
inline void TOffsetExplicit2ClampedBSpline<Degree>::MakeCurves(TArray<TBezierCurve<DimOri, Degree> >& OutCurves, const TSplineBase<DimOri, Degree>& InOriginalSpline) const
{
	TOffsetExplicit2Base<Degree>::MakeCurves(OutCurves, InOriginalSpline);
}
