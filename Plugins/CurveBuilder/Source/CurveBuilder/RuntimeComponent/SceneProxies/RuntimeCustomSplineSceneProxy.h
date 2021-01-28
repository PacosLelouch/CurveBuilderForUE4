// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplinePrimitiveSceneProxy.h"
#include "../RuntimeCustomSplineBaseComponent.h"
//#include "RuntimeCustomSplineSceneProxy.generated.h"

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
struct CURVEBUILDER_API HRuntimeSplineHitProxy : public HRuntimeSplinePrimitiveHitProxy
{
public:
	DECLARE_HIT_PROXY()

	HRuntimeSplineHitProxy(const URuntimeSplinePrimitiveComponent* InComponent)
		: HRuntimeSplinePrimitiveHitProxy(InComponent)
	{}

	virtual EMouseCursor::Type GetMouseCursor() override;
};
#endif

class CURVEBUILDER_API FRuntimeCustomSplineSceneProxy : public FRuntimeSplinePrimitiveSceneProxy
{
protected:

	struct FSpatial3DrawInfo
	{
		FSpatial3DrawInfo(const URuntimeCustomSplineBaseComponent* InComponent)
			: CurveColor(InComponent->CurveColor)
			, CtrlSegColor(InComponent->CtrlSegColor)
			//, CtrlPointColor(InComponent->CtrlPointColor)
			//, SelectedCtrlPointColor(InComponent->SelectedCtrlPointColor)
			//, PointSize(InComponent->DrawPointSize)
			, SegLength(InComponent->DrawSegLength)
			, Thickness(InComponent->DrawThickness)
			, DepthBias(InComponent->DepthBias)
			, bSelected(InComponent->bCustomSelected)
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
			, SplineInternalRef(InComponent->GetSplineProxy() ? InComponent->GetSplineProxy()->Copy() : TSharedRef<FSpatialSplineBase3>())
#else
			, SplineInternalWeakPtr(InComponent->GetSplineProxyWeakPtr())
#endif

			//: SplineComponent(InComponent)
		{}
		FLinearColor CurveColor = FLinearColor::White;
		FLinearColor CtrlSegColor = FLinearColor::White;
		//FLinearColor CtrlPointColor = FLinearColor::White;
		//FLinearColor SelectedCtrlPointColor = FLinearColor::White;
		//float PointSize = 6.f;
		float SegLength = 5.f;
		float Thickness = 0.f;
		float DepthBias = 0.f;
		bool bSelected = false;
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
		TSharedRef<FSpatialSplineBase3> SplineInternalRef;
#else
		TWeakPtr<FSpatialSplineBase3> SplineInternalWeakPtr;
#endif
		//const URuntimeCustomSplineBaseComponent* SplineComponent;
	};

	struct FSpatial3CollisionInfo
	{
		FSpatial3CollisionInfo(const URuntimeCustomSplineBaseComponent* InComponent)
			: DebugCollisionColor(InComponent->DebugCollisionColor)
			, DebugCollisionLineWidth(InComponent->DebugCollisionLineWidth)
			, bDrawDebugCollision(InComponent->bDrawDebugCollision)
			, CollisionLocalToWorld(InComponent->BodyInstance.GetUnrealWorldTransform().ToMatrixWithScale())
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
			, SphylElems(IsValid(InComponent->BodySetup) ? InComponent->BodySetup->AggGeom.SphylElems : TArray<FKSphylElem>())
#else
			, BodySetupPtr(IsValid(InComponent->BodySetup) ? &InComponent->BodySetup : nullptr)
#endif
		{}

		FLinearColor DebugCollisionColor = FLinearColor::White;
		float DebugCollisionLineWidth = 1.f;
		bool bDrawDebugCollision = false;
		FMatrix CollisionLocalToWorld = FMatrix::Identity;
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
		TArray<FKSphylElem> SphylElems;
#else
		UBodySetup* const* BodySetupPtr;
#endif
	};
public:
	//SIZE_T GetTypeHash() const override
	//{
	//	static size_t UniquePointer;
	//	return reinterpret_cast<size_t>(&UniquePointer);
	//}

	FRuntimeCustomSplineSceneProxy(const URuntimeCustomSplineBaseComponent* InComponent)
		: FRuntimeSplinePrimitiveSceneProxy(InComponent)
		, ProxyDrawInfo(InComponent)
		, ProxyCollisionInfo(InComponent)
		//, SplineComponent(InComponent)
	{}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	//virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	//{
	//	FPrimitiveViewRelevance Result;
	//	Result.bDrawRelevance = IsValid(SplineComponent) && IsShown(View)
	//		&& (View->Family->EngineShowFlags.Lighting
	//			|| View->Family->EngineShowFlags.Wireframe
	//			|| View->Family->EngineShowFlags.Splines
	//			|| View->Family->EngineShowFlags.Collision); //&& IsShown(View);//bDrawDebug && !IsSelected() && IsShown(View) && View->Family->EngineShowFlags.Splines;
	//	Result.bDynamicRelevance = true;
	//	Result.bShadowRelevance = IsShadowCast(View);
	//	Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
	//	return Result;
	//}

	//virtual bool CanBeOccluded() const override { return false; }

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof(*this) + GetAllocatedSize(); }
	//uint32 GetAllocatedSize(void) const { return FRuntimeSplinePrimitiveSceneProxy::GetAllocatedSize(); }

public:
	// Static function of draw spline curves.
	//template<int32 Dim = 3, int32 Degree = 3>
	//static void DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const TRuntimeSplineDrawInfo<Dim, Degree>& DrawInfo, const FMatrix& InLocalToWorld, uint8 DepthPriorityGroup);

protected:
	virtual void DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& InLocalToWorld, uint8 DepthPriorityGroup) const;

	virtual void DrawDebugCollisions(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3CollisionInfo& CollisionInfo, uint8 DepthPriorityGroup) const;


	//const URuntimeCustomSplineBaseComponent* SplineComponent;
	FSpatial3DrawInfo ProxyDrawInfo;
	FSpatial3CollisionInfo ProxyCollisionInfo;

	static const bool bDrawLineByCurveLength;
};

