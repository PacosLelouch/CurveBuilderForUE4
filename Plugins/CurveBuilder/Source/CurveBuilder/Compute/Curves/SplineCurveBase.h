// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "Utils/LinearAlgebraUtils.h"
#include "Utils/NumericalCalculationUtils.h"
//#include "Containers/StaticArray.h"

template<int32 Dim, int32 Degree = 3>
class TSplineCurveBase
{
public:
	FORCEINLINE TSplineCurveBase(EForceInit Force = EForceInit::ForceInit) 
	{
		TVecLib<Dim+1>::SetArray(CtrlPoints, 0, Degree + 1);
	}
	FORCEINLINE TSplineCurveBase(const TSplineCurveBase<Dim, Degree>& Curve)
	{
		TVecLib<Dim+1>::CopyArray(CtrlPoints, Curve.CtrlPoints, Degree + 1);
	}
	FORCEINLINE TSplineCurveBase(const TVectorX<Dim>* InPoints)
	{
		for (int32 i = 0; i <= Degree; ++i) {
			for (int32 j = 0; j < Dim; ++j) {
				CtrlPoints[i][j] = InPoints[i][j];
			}
			TVecLib<Dim+1>::Last(CtrlPoints[i]) = 1.;
		}
	}
	FORCEINLINE TSplineCurveBase(const TArray<TVectorX<Dim> >& InPoints)
	{
		for (int32 i = 0; i <= Degree; ++i) {
			for (int32 j = 0; j < Dim; ++j) {
				CtrlPoints[i][j] = InPoints[i][j];
			}
			TVecLib<Dim+1>::Last(CtrlPoints[i]) = 1.;
		}
	}
	FORCEINLINE TSplineCurveBase(const TArray<TVectorX<Dim+1>>& InPoints)
	{
		TVecLib<Dim+1>::CopyArray(CtrlPoints, InPoints.GetData(), Degree + 1);
		//FMemory::Memcpy(CtrlPoints, InPoints, (Degree + 1) * sizeof(TVectorX<Dim+1>));
	}
	FORCEINLINE TSplineCurveBase(const TVectorX<Dim+1>* InPoints)
	{
		TVecLib<Dim+1>::CopyArray(CtrlPoints, InPoints, Degree + 1);
		//FMemory::Memcpy(CtrlPoints, InPoints, (Degree + 1) * sizeof(TVectorX<Dim+1>));
	}
	virtual ~TSplineCurveBase() {}
	FORCEINLINE void Reset(const TVectorX<Dim+1>* InPoints)
	{
		if (InPoints) {
			TVecLib<Dim+1>::CopyArray(CtrlPoints, InPoints, Degree + 1);
		}
		else {
			TVecLib<Dim+1>::SetArray(CtrlPoints, 0, Degree + 1);
		}
	}
	FORCEINLINE TSplineCurveBase<Dim, Degree>& operator=(const TSplineCurveBase<Dim, Degree>& Curve)
	{
		TVecLib<Dim+1>::CopyArray(CtrlPoints, Curve.CtrlPoints, Degree + 1);
		return *this;
	}

	FORCEINLINE void Reverse()
	{
		Algo::Reverse(CtrlPoints, Degree + 1);
	}

	FORCEINLINE static constexpr int32 CurveDim() { return Dim; }
	FORCEINLINE static constexpr int32 CurveDimHomogeneous() { return Dim + 1; }
	FORCEINLINE static constexpr int32 CurveDegree() { return Degree; }
	FORCEINLINE static constexpr int32 CurveOrder() { return Degree + 1; }

	FORCEINLINE F_Box2 GetBox2D(const F_Mat& ProjectMatrix) const
	{
		F_Box2 Box(EForceInit::ForceInit);
		for (int32 i = 0; i <= Degree; ++i) {
			F_Vec3 P = ProjectMatrix.TransformPosition(TVecLib<Dim+1>::Projection(CtrlPoints[i]));
			Box += (const F_Vec2&)P;
		}
		return Box;
	}
	FORCEINLINE F_Box2 GetBox2D() const
	{
		F_Box2 Box(EForceInit::ForceInit);
		for (int32 i = 0; i <= Degree; ++i) {
			Box += (const F_Vec2&)TVecLib<Dim+1>::Projection(CtrlPoints[i]);
		}
		return Box;
	}
	FORCEINLINE F_Box3 GetBox(const F_Mat& ProjectMatrix) const
	{
		F_Box3 Box(EForceInit::ForceInit);
		for (int32 i = 0; i <= Degree; ++i) {
			F_Vec3 P = ProjectMatrix.TransformPosition(TVecLib<Dim+1>::Projection(CtrlPoints[i]));
			Box += P;
		}
		return Box;
	}
	FORCEINLINE F_Box3 GetBox() const
	{
		F_Box3 Box(EForceInit::ForceInit);
		for (int32 i = 0; i <= Degree; ++i) {
			F_Vec3 P = TVecLib<Dim+1>::Projection(CtrlPoints[i]);
			Box += P;
		}
		return Box;
	}
	FORCEINLINE bool IsSmallEnough() const
	{
		return TVecLib<Dim+1>::Projection(CtrlPoints[0]).Equals(TVecLib<Dim+1>::Projection(CtrlPoints[Degree]), 0.01);
	}
	FORCEINLINE TVectorX<Dim> Center() const
	{
		return (TVecLib<Dim+1>::Projection(CtrlPoints[0]) + TVecLib<Dim+1>::Projection(CtrlPoints[Degree])) * 0.5;
	}
	FORCEINLINE TVectorX<Dim+1> CenterHomogeneous() const
	{
		return (CtrlPoints[0] + CtrlPoints[Degree]) * 0.5;
	}
	FORCEINLINE void SetPoint(int32 i, const TVectorX<Dim>& P, double Weight = 1.) 
	{
		for (int32 j = 0; j < Dim; ++j) {
			CtrlPoints[i][j] = P[j];
		}
		TVecLib<Dim+1>::Last(CtrlPoints[i]) = Weight;
	}
	FORCEINLINE TVectorX<Dim> GetPoint(int32 i) const { return TVecLib<Dim+1>::Projection(CtrlPoints[i]); }
	FORCEINLINE void SetPointHomogeneous(int32 i, const TVectorX<Dim+1>& P)
	{
		CtrlPoints[i] = P;
	}
	FORCEINLINE TVectorX<Dim+1> GetPointHomogeneous(int32 i) const { return CtrlPoints[i]; }
	FORCEINLINE TVectorX<Dim> GetNormalizedTangent(double T) const { return GetTangent(T).GetSafeNormal(); }

	double GetLength(double T) const
	{
		// if (Degree < 5) 
		TGaussLegendre<NumericalCalculationConst::GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, 0., 1.);
		return GaussLegendre.Integrate(T);
	}

	double GetParamAtLength(double S) const
	{
		// if (Degree < 5) 
		TGaussLegendre<NumericalCalculationConst::GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, 0., 1.);
		return GaussLegendre.SolveFromIntegration(S);
	}

public:
	virtual bool FindParamByPosition(double& OutParam, const TVectorX<Dim>& InPos, double ToleranceSqr = 1.) const;
	virtual bool FindParamsByComponentValue(TArray<double>& OutParams, double InValue, int32 InComponentIndex = 0, double ToleranceSqr = 1.) const;

public:
	virtual TVectorX<Dim> GetPosition(double T) const = 0;
	virtual TVectorX<Dim> GetTangent(double T) const = 0;
	virtual double GetPlanCurvature(double T, int32 PlanIndex = 0) const = 0;
	virtual double GetCurvature(double T) const = 0;
	virtual void ToPolynomialForm(TVectorX<Dim+1>* OutPolyForm) const = 0;
	virtual void CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& OutHodograph) const = 0;
	virtual void ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1, 0)>& InCurve) = 0;

protected:
	// Homogeneous
	TVectorX<Dim+1> CtrlPoints[Degree + 1];
};

#include "SplineCurveBase.inl"
