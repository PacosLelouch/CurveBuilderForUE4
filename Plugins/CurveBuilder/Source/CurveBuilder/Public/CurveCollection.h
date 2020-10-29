// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "Curves/PolynomialCurve.h"
#include "Curves/BezierCurve.h"
#include "Curves/RationalBezierCurve.h"
#include "CoreMinimal.h"
#include "CurveCollection.generated.h"

UENUM()
enum class ECurveType : uint8
{
	Unknown,
	Polynomial,
	Bezier,
	RationalBezier,
	// There is always a place holder.
	PlaceHolder,
};
