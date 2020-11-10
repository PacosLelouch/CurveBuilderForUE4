// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "Continuity.h"

bool Continuity::IsGeometric(EEndPointContinuity C)
{
	return GeometricContinuityArray.Contains(C);
}
