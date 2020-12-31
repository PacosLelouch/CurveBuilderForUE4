// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "PrimitiveSceneProxy.h"
#include "HitProxies.h"
#include "../RuntimeSplinePrimitiveComponent.h"
#if WITH_EDITOR
#include "ComponentVisualizer.h"
#endif
//#include "RuntimeSplinePrimitiveSceneProxy.generated.h"

#define CUSTOM_SPLINE_USES_CUSTOM_OCCLUSION_DISTANCE 0
#define DISABLE_COPY_IN_SPLINE_SCENE_PROXY 1
#define ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME 1

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
#include "EngineUtils.h"
#include "PrimitiveSceneInfo.h"
struct CURVEBUILDER_API HRuntimeSplinePrimitiveHitProxy : 
//#if WITH_EDITOR
//	public HComponentVisProxy
//#else
	public HActor
//#endif
{
public:
	DECLARE_HIT_PROXY()
	HRuntimeSplinePrimitiveHitProxy(const URuntimeSplinePrimitiveComponent* InComponent, EHitProxyPriority InPriority = HPP_Wireframe)
//#if WITH_EDITOR
//		: HComponentVisProxy(InComponent, InPriority)
//#else
		: HActor(InComponent->GetAttachmentRootActor(), InComponent, InPriority)
//#endif
		, ComponentWeakPtr(InComponent)
	{}

	virtual EMouseCursor::Type GetMouseCursor() override;

	virtual FRuntimeSplineCommandHelperBase* GetCommandHelper() const;

	TWeakObjectPtr<const URuntimeSplinePrimitiveComponent> ComponentWeakPtr;
};
#endif

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
		, bDrawInGameOverride(InComponent->bDrawInGame)
		, ComponentWeakPtr(InComponent)
	{}

	bool IsComponentVaild() const { return ComponentWeakPtr.IsValid() && IsValid(ComponentWeakPtr.Get()) && !ComponentWeakPtr->IsBeingDestroyed(); }

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;

	virtual bool CanBeOccluded() const override { return false; }

	virtual bool IsDrawnInGame() const override { return bDrawInGameOverride; }

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
	virtual HHitProxy* CreateHitProxies(UPrimitiveComponent* Component, TArray<TRefCountPtr<HHitProxy> >& OutHitProxies) override
	{
		HHitProxy* HitProxy = nullptr;
		if (URuntimeSplinePrimitiveComponent* CastedComponent = Cast<URuntimeSplinePrimitiveComponent>(Component))
		{
			HitProxy = new HRuntimeSplinePrimitiveHitProxy(CastedComponent);
			OutHitProxies.Add(HitProxy);
			return HitProxy;
		}

		HitProxy = FPrimitiveSceneProxy::CreateHitProxies(Component, OutHitProxies);
		return HitProxy;
	}
#endif

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
#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
		PDI->SetHitProxy(nullptr);
#endif
		RenderBounds(PDI, ViewFamily.EngineShowFlags, BoundsForRender, IsSelected());
	}

private:

	bool bDrawInGameOverride;

protected:
	TWeakObjectPtr<const URuntimeSplinePrimitiveComponent> ComponentWeakPtr;

	static const FVector CollisionScale3D;
	static const int32 DrawCollisionSides;

	static void DrawHalfCircle(FPrimitiveDrawInterface* PDI, const FVector& Base, const FVector& X, const FVector& Y, const FColor Color, float Radius, uint8 DepthPriorityGroup, float Thickness = 0.f);
};

