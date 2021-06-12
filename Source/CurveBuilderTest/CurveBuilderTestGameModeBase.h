// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CGDemoGameModeBase.h"
#include "CurveCollection.h"
#include "CurveBuilderTestGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class CURVEBUILDERTEST_API ACurveBuilderTestGameModeBase : public ACGDemoGameModeBase
{
	GENERATED_BODY()
public:
	ACurveBuilderTestGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	TPolynomialCurve<3, 3> Poly3;
	TPolynomialCurve<2, 3> Poly2;
	TPolynomialCurve<1, 3> Poly1;
	TBezierCurve<3, 3> Bezier3;
	TBezierCurve<2, 3> Bezier2;
	TBezierCurve<1, 3> Bezier1;
	TRationalBezierCurve<3, 3> RationalBezier3;
	TRationalBezierCurve<2, 3> RationalBezier2;
	TRationalBezierCurve<1, 3> RationalBezier1;
};
