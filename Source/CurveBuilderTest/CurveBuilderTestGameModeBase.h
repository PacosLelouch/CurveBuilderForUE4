// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PolynomialCurve.h"
#include "BezierCurve.h"
#include "CurveBuilderTestGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class CURVEBUILDERTEST_API ACurveBuilderTestGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	TPolynomialCurve<3> Poly3;
	TPolynomialCurve<2> Poly2;
	TBezierCurve<3> Bezier3;
	TBezierCurve<2> Bezier2;
};
