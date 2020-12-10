// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "Components/PrimitiveComponent.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeSplinePointBaseComponent.generated.h"

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Rendering, Mobile))
class CURVEBUILDER_API URuntimeSplinePointBaseComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:

};
