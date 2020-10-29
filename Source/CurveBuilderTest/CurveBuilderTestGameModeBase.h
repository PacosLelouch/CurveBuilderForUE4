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
	TPolynomialCurve<3> Poly3;
	TPolynomialCurve<2> Poly2;
	TBezierCurve<3> Bezier3;
	TBezierCurve<2> Bezier2;
	TRationalBezierCurve<3> RationalBezier3;
	TRationalBezierCurve<2> RationalBezier2;
};
