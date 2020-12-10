// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "../Splines/BSpline.h"
#include "OffsetExplicitBase.h"


template<int32 Degree>
class TOffsetExplicit2ClampedBSpline : public TOffsetExplicit2Base<Degree>, public TClampedBSpline<2, Degree>
{
public:
	using TOffsetExplicit2Base<Degree>::TOffsetExplicit2Base;

	virtual void GetKnotsS(TArray<double>& OutKnotsS) const override;

	template<int32 DimOri>
	virtual void MakeCurves(TArray<TBezierCurve<DimOri, Degree> >& OutCurves, const TSplineBase<DimOri, Degree>& InOriginalSpline) const override;

protected:

};

#include "OffsetExplicitBSpline.inl"
