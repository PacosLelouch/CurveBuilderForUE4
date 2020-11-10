// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"

enum class EEndPointContinuity : uint8
{
	C0,
	G1,
	C1,
	G2,
	C2,
};

namespace Continuity
{
	const TArray<EEndPointContinuity> GeometricContinuityArray{
		EEndPointContinuity::G1,
		EEndPointContinuity::G2,
	};

	bool CURVEBUILDER_API IsGeometric(EEndPointContinuity C);
}