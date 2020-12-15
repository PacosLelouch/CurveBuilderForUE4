// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"
#include "../RuntimeSplinePrimitiveComponent.h"

#define CUSTOM_SPLINE_USES_CUSTOM_OCCLUSION_DISTANCE 0
#define DISABLE_COPY_IN_SPLINE_SCENE_PROXY 1

class FRuntimeSplinePrimitiveSceneProxy : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FRuntimeSplinePrimitiveSceneProxy(const URuntimeSplinePrimitiveComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, ComponentForCheck(InComponent)
	{}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	virtual HHitProxy* CreateHitProxies(UPrimitiveComponent* Component, TArray<TRefCountPtr<HHitProxy> >& OutHitProxies) override
	{
		// Not implemented yet.
		return FPrimitiveSceneProxy::CreateHitProxies(Component, OutHitProxies);
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

	virtual void ApplyLateUpdateTransform(const FMatrix& LateUpdateTransform) override
	{
		FPrimitiveSceneProxy::ApplyLateUpdateTransform(LateUpdateTransform);
	}
#endif

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof(*this) + GetAllocatedSize(); }
	//uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

public:
	void DrawBoundsIfNecessary(FPrimitiveDrawInterface* PDI, const FSceneViewFamily& ViewFamily) const
	{
#if CUSTOM_SPLINE_USES_CUSTOM_OCCLUSION_DISTANCE
		FBoxSphereBounds BoundsForRender = GetCustomOcclusionBounds();
#else
		FBoxSphereBounds BoundsForRender = GetBounds();
#endif
		RenderBounds(PDI, ViewFamily.EngineShowFlags, BoundsForRender, IsSelected());
	}

protected:
	const UPrimitiveComponent* ComponentForCheck;

	static const FVector CollisionScale3D;
	static const int32 DrawCollisionSides;

	static void DrawHalfCircle(FPrimitiveDrawInterface* PDI, const FVector& Base, const FVector& X, const FVector& Y, const FColor Color, float Radius, uint8 DepthPriorityGroup, float Thickness = 0.f);
};

