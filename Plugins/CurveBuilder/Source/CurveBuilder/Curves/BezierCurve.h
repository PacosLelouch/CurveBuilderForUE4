// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineCurveBase.h"

template<int32 Dim, int32 Degree = 3>
class TBezierCurve : public TSplineCurveBase<Dim, Degree>
{
public:
	using TSplineCurveBase::TSplineCurveBase;

	virtual TVectorX<Dim> GetPosition(double T) const override;
	virtual TVectorX<Dim> GetTangent(double T) const override;
	virtual double GetPrincipalCurvature(double T, int32 Principal = 0) const override;
	virtual double GetCurvature(double T) const override;
	virtual void ToPolynomialForm(TVectorX<Dim+1>* OutPolyForm) const override;
	virtual void CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1)>& OutHodograph) const override;
	virtual void ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1)>& InCurve) const override;

public:
	void Split(TBezierCurve<Dim, Degree>& OutFirst, TBezierCurve<Dim, Degree>& OutSecond, double T = 0.5);

	TVectorX<Dim> GetPositionDirectly(double T) const;
	TVectorX<Dim> GetPositionIteratively(double T) const;
};


#include "BezierCurve.inl"
