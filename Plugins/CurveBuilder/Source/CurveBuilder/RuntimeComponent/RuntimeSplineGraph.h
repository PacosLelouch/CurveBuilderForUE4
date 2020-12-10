// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeSplineGraph.generated.h"

using FSpatialSplineGraph3 = typename TSplineGraph<3, 3>;
class URuntimeCustomSplineBaseComponent;

UCLASS(BlueprintType)
class CURVEBUILDER_API URuntimeSplineGraph : public UObject
{
	GENERATED_BODY()
public:


public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline")
	void GetOwningSplines(TArray<URuntimeCustomSplineBaseComponent*>& Splines);

public:
	FSpatialSplineGraph3 SplineGraphProxy;
	TMap<TSharedPtr<FSpatialSplineGraph3::FSplineWrapper>, URuntimeCustomSplineBaseComponent*> SplineComponentMap;
};