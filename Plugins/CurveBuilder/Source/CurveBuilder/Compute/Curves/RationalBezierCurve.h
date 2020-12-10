// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "SplineCurveBase.h"

// Rational Bezier curve with weight. The format of each control point: (x1*w, x2*w, ..., xn*w, w)
// 1. Rational Bezier curves can represent conic sections exactly.
// 2. If you perform a perspective projection of a Bezier curve (either a polynomial or rational), the result is a rational Bezier curve.
// 3. Rational Bezier curves permit some additional shape control : by changing the weights, you change the shape of the curve.
// 4. It is possible to re parametrize the curve by simply changing the weights in a specific manner.
template<int32 Dim, int32 Degree = 3>
class TRationalBezierCurve : public TSplineCurveBase<Dim, Degree>
{
public:
	using TSplineCurveBase::TSplineCurveBase;

	virtual TVectorX<Dim> GetPosition(double T) const override;
	virtual TVectorX<Dim> GetTangent(double T) const override;
	virtual double GetPlanCurvature(double T, int32 PlanIndex = 0) const override;
	virtual double GetCurvature(double T) const override;
	virtual void ToPolynomialForm(TVectorX<Dim+1> OutPolyForm[Degree + 1]) const override;
	virtual void CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const override;
	virtual void ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& InCurve) override;

public:
	TVectorX<Dim+1> Split(TRationalBezierCurve<Dim, Degree>& OutFirst, TRationalBezierCurve<Dim, Degree>& OutSecond, double T = 0.5) const;

	TVectorX<Dim> Horner(double T) const;
	TVectorX<Dim> DeCasteljau(double T) const;
};


#include "RationalBezierCurve.inl"
