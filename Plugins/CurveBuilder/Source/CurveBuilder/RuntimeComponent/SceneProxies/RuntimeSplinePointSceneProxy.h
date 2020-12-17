// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplinePrimitiveSceneProxy.h"
#include "../RuntimeSplinePointBaseComponent.h"
#include "../RuntimeCustomSplineBaseComponent.h"

class FRuntimeSplinePointSceneProxy final : public FRuntimeSplinePrimitiveSceneProxy
{
private:
	struct FSpatial3DrawInfo
	{
		FSpatial3DrawInfo(const URuntimeSplinePointBaseComponent* InComponent)
			: CtrlPointColor(InComponent->CtrlPointColor)
			, SelectedCtrlPointColor(InComponent->SelectedCtrlPointColor)
			, PointSize(InComponent->DrawPointSize)
			, DepthBias(InComponent->DepthBias)
			, bSelected(InComponent->bSelected)
			, SpType((InComponent->ParentSpline && InComponent->ParentSpline->GetSplineProxy()) ? 
				InComponent->ParentSpline->GetSplineProxy()->GetType() : ESplineType::Unknown)
			, TangentFlag(InComponent->TangentFlag)
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
			, SplinePointInternalRef(InComponent->SplinePointProxy.IsValid() ? InComponent->SplinePointProxy.Pin().Get()->Copy() : TSharedRef<FSpatialControlPoint3>())
#else
			, SplinePointInternalWeakPtr(InComponent->SplinePointProxy)
#endif

			//: SplineComponent(InComponent)
		{}
		FLinearColor CtrlPointColor = FLinearColor::White;
		FLinearColor SelectedCtrlPointColor = FLinearColor::White;
		float PointSize = 6.f;
		float DepthBias = 0.f;
		bool bSelected = false;
		ESplineType SpType = ESplineType::Unknown;
		int32 TangentFlag = 0;
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
		TSharedRef<FSpatialControlPoint3> SplinePointInternalRef;
#else
		TWeakPtr<FSpatialControlPoint3> SplinePointInternalWeakPtr;
#endif
		//const URuntimeCustomSplineBaseComponent* SplineComponent;
	};

	struct FSpatial3CollisionInfo
	{
		FSpatial3CollisionInfo(const URuntimeSplinePointBaseComponent* InComponent)
			: DebugCollisionColor(InComponent->DebugCollisionColor)
			, DebugCollisionLineWidth(InComponent->DebugCollisionLineWidth)
			, bDrawDebugCollision(InComponent->bDrawDebugCollision)
			, CollisionLocalToWorld(InComponent->BodyInstance.GetUnrealWorldTransform().ToMatrixWithScale())
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
			, SphereElems(IsValid(InComponent->BodySetup) ? InComponent->BodySetup->AggGeom.SphereElems : TArray<FKSphereElem>())
#else
			, BodySetupPtr(IsValid(InComponent->BodySetup) ? &InComponent->BodySetup : nullptr)
#endif
		{}

		FLinearColor DebugCollisionColor = FLinearColor::White;
		float DebugCollisionLineWidth = 1.f;
		bool bDrawDebugCollision = false;
		FMatrix CollisionLocalToWorld = FMatrix::Identity;
#if !DISABLE_COPY_IN_SPLINE_SCENE_PROXY
		TArray<FKSphereElem> SphereElems;
#else
		UBodySetup* const* BodySetupPtr;
#endif
	};
public:
	FRuntimeSplinePointSceneProxy(const URuntimeSplinePointBaseComponent* InComponent)
		: FRuntimeSplinePrimitiveSceneProxy(InComponent)
		, DrawInfo(InComponent)
		, CollisionInfo(InComponent)
	{}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof(*this) + GetAllocatedSize(); }

protected:
	static void DrawRuntimeSplinePoint(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup);

	static void DrawDebugCollisions(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3CollisionInfo& CollisionInfo, uint8 DepthPriorityGroup);

	FSpatial3DrawInfo DrawInfo;
	FSpatial3CollisionInfo CollisionInfo;
};
