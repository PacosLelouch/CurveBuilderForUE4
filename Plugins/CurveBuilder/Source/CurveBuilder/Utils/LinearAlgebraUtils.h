// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

//#include "CoreTypes.h"
//#include "CoreFwd.h"
#include "CoreMinimal.h"
#include "Math/TransformCalculus2D.h"

#define CLAMP_DEGREE(Degree,Minimum) ((Degree) > (Minimum) ? (Degree) : (Minimum))

using F_Vec2 = FVector2D;
using F_Vec3 = FVector;
using F_Vec4 = FVector4;
using F_Mat = FMatrix;
using F_Box2 = FBox2D;
using F_Box3 = FBox;
using F_Rotator = FRotator;
using F_Quat = FQuat;
using F_Transform2 = FTransform2D;
using F_Transform3 = FTransform;

// Start Vector

#define GENERATED_VECTOR_LIBRARY_FUNCTIONS(Dim) \
	FORCEINLINE static const auto& Zero() { static const FType ZeroVector = FType(EForceInit::ForceInit); return ZeroVector; } \
	FORCEINLINE static auto& Last(FType& V) { return V[Dim - 1]; } \
	FORCEINLINE static auto Last(const FType& V) { return V[Dim - 1]; } \
	FORCEINLINE static void WeightToOne(FType& V) \
	{ \
		if(!FMath::IsNearlyZero(V[Dim - 1])) \
		{ \
			double InvWeight = 1. / V[Dim - 1]; \
			for(int32 i = 0; i < Dim; ++i) \
			{ \
				V[i] *= InvWeight; \
			} \
		} \
	} \
	FORCEINLINE static void SetArray(FType* Dst, uint8 Byte, SIZE_T Size) \
		{ FMemory::Memset(Dst, Byte, Size * sizeof(FType)); } \
	FORCEINLINE static void CopyArray(FType* Dst, const FType* Src, SIZE_T Size) \
		{ FMemory::Memcpy(Dst, Src, Size * sizeof(FType)); } \
	FORCEINLINE static double PrincipalCurvature(const FType& DP, const FType& DDP, int32 Principal) \
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
	FORCEINLINE static double Curvature(const FType& DP, const FType& DDP) \
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

template<int32 Dim>
struct TVecLib;

template<>
struct TVecLib<2> {
	using FType = F_Vec2;
	using FTypeHomogeneous = F_Vec3;
	GENERATED_VECTOR_LIBRARY_FUNCTIONS(2)

	FORCEINLINE static FTypeHomogeneous Homogeneous(const FType& V, double Weight = 1.)
	{
		if (FMath::IsNearlyZero(Weight)) {
			return FTypeHomogeneous(V.X, V.Y, 0.);
		}
		return FTypeHomogeneous(V.X*Weight, V.Y*Weight, Weight);
	}
};

template<>
struct TVecLib<3> {
	using FType = F_Vec3;
	using FTypeHomogeneous = F_Vec4;
	using FTypeProjection = F_Vec2;
	GENERATED_VECTOR_LIBRARY_FUNCTIONS(3)

	FORCEINLINE static FTypeHomogeneous Homogeneous(const FType& V, double Weight = 1.)
	{
		if (FMath::IsNearlyZero(Weight)) {
			return FTypeHomogeneous(V.X, V.Y, V.Z, 0.);
		}
		return FTypeHomogeneous(V.X*Weight, V.Y*Weight, V.Z*Weight, Weight);
	}

	FORCEINLINE static FTypeProjection Projection(const FType& V)
	{
		double Weight = V[2];
		if (FMath::IsNearlyZero(Weight)) {
			return FTypeProjection(V.X, V.Y);
		}
		double InvWeight = 1. / Weight;
		return FTypeProjection(V.X*InvWeight, V.Y*InvWeight);
	}
};

template<>
struct TVecLib<4> {
	using FType = F_Vec4;
	using FTypeProjection = F_Vec3;
	GENERATED_VECTOR_LIBRARY_FUNCTIONS(4)

	FORCEINLINE static FTypeProjection Projection(const FType& V)
	{
		double Weight = V[3];
		if (FMath::IsNearlyZero(Weight)) {
			return FTypeProjection(V.X, V.Y, V.Z);
		}
		double InvWeight = 1. / Weight;
		return FTypeProjection(V.X*InvWeight, V.Y*InvWeight, V.Z*InvWeight);
	}
};

template<int32 Dim>
using TVectorX = typename TVecLib<Dim>::FType;

// End Vector