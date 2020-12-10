// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/PrimitiveComponent.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeCustomBSplineComponent.generated.h"

using FSpatialClampedBBase3 = typename TClampedBSpline<3, 3>;

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Collision, Lighting, Rendering, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API URuntimeCustomSplineComponent : public URuntimeCustomSplineBaseComponent
{
	GENERATED_BODY()
public:

public:
};

