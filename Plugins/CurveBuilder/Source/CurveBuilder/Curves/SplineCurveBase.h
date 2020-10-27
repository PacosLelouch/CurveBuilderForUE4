// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "LinearAlgebraUtils.h"
#include "NumericalCalculationUtils.h"
#include "Containers/StaticArray.h"
#include "HAL/UnrealMemory.h"

#define CLAMP_DEGREE(Degree) ((Degree) > 0 ? (Degree) : 0)

template<int32 Dim, int32 Degree = 3>
class TSplineCurveBase
{
public:
	FORCEINLINE TSplineCurveBase(EForceInit Force = EForceInit::ForceInit) 
	{
		TVectorX<Dim+1>::SetArray(CtrlPoints, 0, Degree + 1);
		//FMemory::Memset(CtrlPoints, 0, (Degree + 1) * sizeof(TVectorX<Dim + 1>));
	}
	FORCEINLINE TSplineCurveBase(const TVectorX<Dim>* InPoints)
	{
		for (int32 i = 0; i <= Degree; ++i) {
			CtrlPoints[i] = InPoints[i];
			CtrlPoints[i].Last() = 1.;
		}
	}
	FORCEINLINE TSplineCurveBase(const TArray<TVectorX<Dim> >& InPoints)
	{
		for (int32 i = 0; i <= Degree; ++i) {
			CtrlPoints[i] = InPoints[i];
			CtrlPoints[i].Last() = 1.;
		}
	}
	FORCEINLINE TSplineCurveBase(const TArray<TVectorX<Dim + 1> >& InPoints)
	{
		TVectorX<Dim+1>::CopyArray(CtrlPoints, InPoints.GetData(), Degree + 1);
		//FMemory::Memcpy(CtrlPoints, InPoints, (Degree + 1) * sizeof(TVectorX<Dim + 1>));
	}
	FORCEINLINE TSplineCurveBase(const TVectorX<Dim + 1>* InPoints)
	{
		TVectorX<Dim+1>::CopyArray(CtrlPoints, InPoints, Degree + 1);
		//FMemory::Memcpy(CtrlPoints, InPoints, (Degree + 1) * sizeof(TVectorX<Dim + 1>));
	}
	FORCEINLINE int32 CurveDim() const { return Dim; }
	FORCEINLINE int32 CurveDimHomogeneous() const { return Dim + 1; }
	FORCEINLINE int32 CurveDegree() const { return Degree; }
	FORCEINLINE int32 CtrlPointsNum() const { return Degree + 1; }

	FORCEINLINE FBox2 GetBox(const FMat& ProjectMatrix) const
	{
		FBox2 Box(EForceInit::ForceInit);
		for (int32 i = 0; i <= Degree; ++i) {
			FVec3 P = ProjectMatrix.TransformPosition(CtrlPoints[i]);
			Box += (const FVec2&)P;
		}
		return Box;
	}
	FORCEINLINE bool IsSmallEnough() const
	{
		return CtrlPoints[0].NonHomogeneous().Equals(CtrlPoints[Degree].NonHomogeneous(), 0.01);
	}
	FORCEINLINE TVectorX<Dim> Center() const
	{
		return (CtrlPoints[0].NonHomogeneous() + CtrlPoints[Degree].NonHomogeneous()) * 0.5;
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
		CtrlPoints[i].Last() = Weight; 
	}
	FORCEINLINE TVectorX<Dim> GetPoint(int32 i) const { return CtrlPoints[i].NonHomogeneous(); }
	FORCEINLINE void SetPointHomogeneous(int32 i, const TVectorX<Dim+1>& P)
	{
		CtrlPoints[i] = P;
	}
	FORCEINLINE TVectorX<Dim+1> GetPointHomogeneous(int32 i) const { return CtrlPoints[i]; }
	FORCEINLINE TVectorX<Dim> GetNormalizedTangent(double T) const { return TVectorX<Dim>(GetTangent(T).GetSafeNormal()); }

	double GetLength(double T) const
	{
		// if (Degree < 5) 
		TGaussLegendre<GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, 0., 1.);
		return GaussLegendre.Integrate(T);
	}

	double GetParameterAtLength(double S) const
	{
		// if (Degree < 5) 
		TGaussLegendre<GaussLegendreN> GaussLegendre([this](double InT) -> double {
			return GetTangent(InT).Size();
		}, 0., 1.);
		return GaussLegendre.SolveFromIntegration(S);
	}

	virtual TVectorX<Dim> GetPosition(double T) const = 0;
	virtual TVectorX<Dim> GetTangent(double T) const = 0;
	virtual double GetPrincipalCurvature(double T, int32 Principal = 0) const = 0;
	virtual double GetCurvature(double T) const = 0;
	virtual void ToPolynomialForm(TVectorX<Dim + 1>* OutPolyForm) const = 0;
	virtual void CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1)>& OutHodograph) const = 0;
	virtual void ElevateFrom(const TSplineCurveBase<Dim, CLAMP_DEGREE(Degree-1)>& InCurve) const = 0;

protected:
	// Homogeneous
	TVectorX<Dim+1> CtrlPoints[Degree + 1];
};

#include "SplineCurveBase.inl"
