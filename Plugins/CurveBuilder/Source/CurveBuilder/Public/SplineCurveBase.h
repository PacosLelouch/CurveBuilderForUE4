// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

//#include "CoreTypes.h"
//#include "CoreFwd.h"
#include "CoreMinimal.h"
#include "Containers/StaticArray.h"
#include "HAL/UnrealMemory.h"

#define CLAMP_DEGREE(Degree) ((Degree) > 0 ? (Degree) - 1 : 0)

using FVec2 = FVector2D;
using FVec3 = FVector;
using FVec4 = FVector4;
using FMat = FMatrix;
using FBox2 = FBox2D;
using FBox3 = FBox;

// Start Vector

#define GENERATED_TEMPLATE_VECTOR_BODY(Dim) \
	using FVec##Dim::FVec##Dim; \
	FORCEINLINE TVectorX() : FVec##Dim(EForceInit::ForceInit) {} \
	FORCEINLINE explicit TVectorX(EForceInit Force) : FVec##Dim(Force) {} \
	FORCEINLINE TVectorX(const FVec##Dim& TV) : FVec##Dim(TV) {} \
	FORCEINLINE TVectorX(const TVectorX& TV) : FVec##Dim((const FVec##Dim&)TV) {} \
	FORCEINLINE TVectorX& operator=(const TVectorX& TV) { (FVec##Dim&)*this = (const FVec##Dim&)TV; return *this; } \
	FORCEINLINE operator FVec##Dim&(){ return (FVec##Dim&)*this; } \
	FORCEINLINE operator const FVec##Dim&() const { return (const FVec##Dim&)*this; } \
	FORCEINLINE operator FVec##Dim() const { return (FVec##Dim)*this; } \
	FORCEINLINE auto& Component(int32 i){ return *((&X) + i); } \
	FORCEINLINE auto Component(int32 i) const { return (&X)[i]; } \
	FORCEINLINE auto& operator[](int32 i){ return (&X)[i]; } \
	FORCEINLINE auto operator[](int32 i) const { return (&X)[i]; } \
	FORCEINLINE auto& Last() { return (&X)[Dim - 1]; } \
	FORCEINLINE auto Last() const { return (&X)[Dim - 1]; } \
	FORCEINLINE void WeightToOne() \
	{ \
		if(!FMath::IsNearlyZero(Last())) \
		{ \
			double InvWeight = 1. / Last(); \
			for(int32 i = 0; i < Dim; ++i) \
			{ \
				Component(i) *= InvWeight; \
			} \
		} \
	} \
	FORCEINLINE static void SetArray(TVectorX<Dim>* Dst, uint8 Byte, SIZE_T Size) \
		{ FMemory::Memset(Dst, Byte, Size * sizeof(TVectorX<Dim>)); } \
	FORCEINLINE static void CopyArray(TVectorX<Dim>* Dst, const TVectorX<Dim>* Src, SIZE_T Size) \
		{ FMemory::Memcpy(Dst, Src, Size * sizeof(TVectorX<Dim>)); } \
	FORCEINLINE static double PrincipalCurvature(const TVectorX<Dim>& DP, const TVectorX<Dim>& DDP, int32 Principal) \
	{ \
		double Nu = 0., De = 0.; \
		int32 i = Principal % Dim; \
		int32 j = (i + 1) % Dim; \
		De += DP[i] * DP[i] + DP[j] * DP[j]; \
		double Determinant = DP[i] * DDP[j] - DP[j] * DDP[i]; \
		Nu += Determinant * Determinant; \
		Nu = sqrt(Nu); \
		if (FMath::IsNearlyZero(De)) { return 0.; } \
		De = sqrt(De); \
		De = De * De * De; \
		return Nu / De;  \
	} \
	/** https://www.zhihu.com/question/356547555?sort=created */ \
	FORCEINLINE static double Curvature(const TVectorX<Dim>& DP, const TVectorX<Dim>& DDP) \
	{ \
		double Nu = 0., De = 0.; \
		for(int32 i = 0; i < Dim; ++i) \
		{ \
			int32 j = (i + 1) % Dim; \
			De += DP[i] * DP[i]; \
			double Determinant = DP[i] * DDP[j] - DP[j] * DDP[i]; \
			Nu += Determinant * Determinant; \
		} \
		if (FMath::IsNearlyZero(De)) { return 0.; } \
		Nu = sqrt(Nu); \
		De = sqrt(De); \
		De = De * De * De; \
		return Nu / De;  \
	} 


//FORCEINLINE TVectorX& operator=(const TVectorX<Dim-1>& TV) { (FVec##Dim&)*this = (const FVec##Dim&)TV; (&this->X)[Dim - 1] = 1.; return *this; } \


template<int32 Dim>
struct TVectorX;

template <int32 Dim> 
struct TIsPODType<TVectorX<Dim> > { enum { Value = true }; };

template<>
struct TVectorX<2> : public FVec2
{
	GENERATED_TEMPLATE_VECTOR_BODY(2)
	FORCEINLINE TVectorX<3> Homogeneous(double Weight = 1.) const;
};
template<>
struct TVectorX<3> : public FVec3
{
	GENERATED_TEMPLATE_VECTOR_BODY(3)
	FORCEINLINE TVectorX<2> NonHomogeneous() const
	{
		double Weight = Last();
		if (FMath::IsNearlyZero(Weight)) {
			return TVectorX<2>(X, Y);
		}
		double InvWeight = 1. / Weight;
		return TVectorX<2>(X*InvWeight, Y*InvWeight);
	}
	FORCEINLINE TVectorX<4> Homogeneous(double Weight = 1.) const;
};
template<>
struct TVectorX<4> : public FVec4
{
	GENERATED_TEMPLATE_VECTOR_BODY(4)
	FORCEINLINE TVectorX<3> NonHomogeneous() const
	{
		double Weight = Last();
		if (FMath::IsNearlyZero(Weight)) {
			return TVectorX<3>(X, Y, Z);
		}
		double InvWeight = 1. / Weight;
		return TVectorX<3>(X*InvWeight, Y*InvWeight, Z*InvWeight);
	}
}; 

TVectorX<3> TVectorX<2>::Homogeneous(double Weight) const
{
	if (FMath::IsNearlyZero(Weight)) {
		return TVectorX<3>(X, Y, 0.);
	}
	return TVectorX<3>(X*Weight, Y*Weight, Weight);
}

TVectorX<4> TVectorX<3>::Homogeneous(double Weight) const
{
	if (FMath::IsNearlyZero(Weight)) {
		return TVectorX<4>(X, Y, Z, 0.);
	}
	return TVectorX<4>(X*Weight, Y*Weight, Z*Weight, Weight);
}

// End Vector

// Start Curve Base

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

	virtual TVectorX<Dim> GetPosition(double T) const = 0;
	virtual TVectorX<Dim> GetTangent(double T) const = 0;
	virtual double GetPrincipalCurvature(double T, int32 Principal = 0) const = 0;
	virtual double GetCurvature(double T) const = 0;
	virtual void ToPolynomialForm(TVectorX<Dim + 1>* OutPolyForm) const = 0;
	virtual void CreateHodograph(TSplineCurveBase<Dim, CLAMP_DEGREE(Degree)>& OutHodograph) const = 0;

protected:
	// Homogeneous
	TVectorX<Dim+1> CtrlPoints[Degree + 1];
};

#include "SplineCurveBase.inl"

// End Curve Base
