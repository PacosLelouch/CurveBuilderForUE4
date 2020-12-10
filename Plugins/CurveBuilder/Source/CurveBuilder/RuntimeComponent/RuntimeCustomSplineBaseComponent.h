// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "Components/PrimitiveComponent.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.generated.h"

using FSpatialSplineBase3 = typename TSplineBase<3, 3>;

template<int32 Dim = 3, int32 Degree = 3>
struct TRuntimeSplineDrawInfo;

using FSpatial3DrawInfo = typename TRuntimeSplineDrawInfo<3, 3>;

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Rendering, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API URuntimeCustomSplineBaseComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

public:

	FSpatialSplineGraph3::FSplineType* GetSplineProxy() const;

	template<typename T>
	T* GetSplineProxyInternal() const
	{
		if (SplineBaseWrapperProxy.Get() && SplineBaseWrapperProxy.Get()->Spline.Get())
		{
			return static_cast<T*>(SplineBaseWrapperProxy.Get()->Spline.Get());
		}
		return nullptr;
	}

	// Static function of draw spline curves.
	//template<int32 Dim = 3, int32 Degree = 3>
	//static void DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const TRuntimeSplineDrawInfo<Dim, Degree>& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup);
	
	// Static function of draw spline curves.
	static void DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	bool bSelected = true;

	// Be parameter length or curve length?
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float SegLength = 0.05f;//10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float Thickness = 2.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float DepthBias = 0.f;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	//int32 PointSize = 6;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor LineColor = FLinearColor(0.1f, 0.7f, 1.f);

public:
	TSharedPtr<FSpatialSplineGraph3::FSplineWrapper> SplineBaseWrapperProxy;

};
