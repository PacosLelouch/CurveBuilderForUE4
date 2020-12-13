// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"
#include "RuntimeCustomSplineBaseComponent.h"

#define CUSTOM_SPLINE_USES_CUSTOM_OCCLUSION_DISTANCE 0

class FRuntimeCustomSplineSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FRuntimeCustomSplineSceneProxy(const URuntimeCustomSplineBaseComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, SplineComponent(InComponent)
	{}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeCustomSplineSceneProxy_GetDynamicMeshElements);

		if (!IsValid(SplineComponent))
		{
			return;
		}

		auto* SplineProxy = SplineComponent->GetSplineProxy();
		if (!SplineProxy)
		{
			return;
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

				const FMatrix& LocalToWorldMat = GetLocalToWorld();
				//const FMatrix& LocalToWorldMat = SplineComponent->GetSplineLocalToWorldMatrix();

				// Taking into account the min and maximum drawing distance
				const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - LocalToWorldMat.GetOrigin()).SizeSquared();
				if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
				{
					continue;
				}

				FSpatial3DrawInfo DrawInfo(SplineComponent);

				URuntimeCustomSplineBaseComponent::DrawRuntimeSpline(
					PDI, View,
					DrawInfo, LocalToWorldMat,
					SplineComponent->DepthPriorityGroup);

				if (SplineComponent->bDrawDebugCollision)
				{
					SplineComponent->DrawDebugCollisions(
						SplineComponent,
						PDI, View,
						SplineComponent->BodyInstance.GetUnrealWorldTransform().ToMatrixWithScale(),
						SplineComponent->DepthPriorityGroup);
				}

#if CUSTOM_SPLINE_USES_CUSTOM_OCCLUSION_DISTANCE
				FBoxSphereBounds BoundsForRender = GetCustomOcclusionBounds();
#else
				FBoxSphereBounds BoundsForRender = GetBounds();
#endif
				RenderBounds(PDI, ViewFamily.EngineShowFlags, BoundsForRender, IsSelected());
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsValid(SplineComponent) && IsShown(View)
			&& (View->Family->EngineShowFlags.Lighting
				|| View->Family->EngineShowFlags.Wireframe
				|| View->Family->EngineShowFlags.Splines
				|| View->Family->EngineShowFlags.Collision); //&& IsShown(View);//bDrawDebug && !IsSelected() && IsShown(View) && View->Family->EngineShowFlags.Splines;
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}

	virtual bool CanBeOccluded() const override { return false; }

#if CUSTOM_SPLINE_USES_CUSTOM_OCCLUSION_DISTANCE
	virtual FBoxSphereBounds GetCustomOcclusionBounds() const override
	{
		FBoxSphereBounds CustomBounds = GetBounds();
		return CustomBounds;
	}

#if WITH_EDITORONLY_DATA
	bool GetPrimitiveDistance(int32 LODIndex, int32 SectionIndex, const FVector& ViewOrigin, float& PrimitiveDistance) const override
	{
		const bool bUseNewMetrics = CVarStreamingUseNewMetrics.GetValueOnRenderThread() != 0;

		const FBoxSphereBounds& PrimBounds = GetCustomOcclusionBounds();

		FVector ViewToObject = PrimBounds.Origin - ViewOrigin;

		float DistSqMinusRadiusSq = 0;
		if (bUseNewMetrics)
		{
			ViewToObject = ViewToObject.GetAbs();
			FVector BoxViewToObject = ViewToObject.ComponentMin(PrimBounds.BoxExtent);
			DistSqMinusRadiusSq = FVector::DistSquared(BoxViewToObject, ViewToObject);
		}
		else
		{
			float Distance = ViewToObject.Size();
			DistSqMinusRadiusSq = FMath::Square(Distance) - FMath::Square(PrimBounds.SphereRadius);
		}

		PrimitiveDistance = FMath::Sqrt(FMath::Max<float>(1.f, DistSqMinusRadiusSq));
		return true;
	}
#endif

	virtual void OnTransformChanged() override
	{
		FPrimitiveSceneProxy::OnTransformChanged();
	}

	virtual void ApplyLateUpdateTransform(const FMatrix& LateUpdateTransform)
	{
		FPrimitiveSceneProxy::ApplyLateUpdateTransform(LateUpdateTransform);
	}
#endif

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof *this + GetAllocatedSize(); }
	uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

protected:
	const URuntimeCustomSplineBaseComponent* SplineComponent;
};

#undef CUSTOM_SPLIN_USES_CUSTOM_OCCLUSION_DISTANCE
