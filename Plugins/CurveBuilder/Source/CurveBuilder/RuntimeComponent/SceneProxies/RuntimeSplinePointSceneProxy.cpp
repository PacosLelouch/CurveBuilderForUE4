// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplinePointSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
IMPLEMENT_HIT_PROXY(HRuntimeSplinePointHitProxy, HRuntimeSplinePrimitiveHitProxy)
#endif

void FRuntimeSplinePointSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeSplinePointSceneProxy_GetDynamicMeshElements);

	for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if (VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];
			FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

			const FMatrix& LocalToWorldMat = GetLocalToWorld();

			// Taking into account the min and maximum drawing distance
			const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - LocalToWorldMat.GetOrigin()).SizeSquared();
			if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
			{
				continue;
			}

			int32 CurrentDepthPriorityGroup = GetStaticDepthPriorityGroup();

			DrawRuntimeSplinePoint(
				PDI, View,
				ProxyDrawInfo, LocalToWorldMat,
				CurrentDepthPriorityGroup);

			DrawDebugCollisions(
				PDI, View,
				ProxyCollisionInfo,
				CurrentDepthPriorityGroup);

			DrawBoundsIfNecessary(PDI, ViewFamily);
		}
	}
}

void FRuntimeSplinePointSceneProxy::DrawRuntimeSplinePoint(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& InLocalToWorld, uint8 DepthPriorityGroup) const
{
	if (!View || !PDI)
	{
		return;
	}

	SCOPE_MUTEX_LOCK(ComponentWeakPtr->RenderMuteX);
#if DISABLE_COPY_IN_SPLINE_SCENE_PROXY
	if (!DrawInfo.SplinePointInternalWeakPtr.IsValid())
	{
		return;
	}
	const FSpatialControlPoint3& SplinePointInternal = *DrawInfo.SplinePointInternalWeakPtr.Pin().Get();
#else
	const FSpatialControlPoint3& SplinePointInternal = *DrawInfo.SplinePointInternalRef.Get();
#endif

	FLinearColor UseColor = DrawInfo.bSelected ? DrawInfo.SelectedCtrlPointColor : DrawInfo.CtrlPointColor;
	FVector Pos = FVector::ZeroVector;

	switch (DrawInfo.SpType)
	{
	case ESplineType::ClampedBSpline:
	{
		const auto& BSplineControlPoint = static_cast<const TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FControlPointType&>(SplinePointInternal);
		Pos = InLocalToWorld.TransformPosition(BSplineControlPoint.Pos);
	}
	break;
	case ESplineType::BezierString:
	{
		const auto& BeziersControlPoint = static_cast<const TSplineTraitByType<ESplineType::BezierString, 3, 3>::FControlPointType&>(SplinePointInternal);
		if (DrawInfo.TangentFlag == 0)
		{
			Pos = InLocalToWorld.TransformPosition(BeziersControlPoint.Pos);
		}
		else if (DrawInfo.TangentFlag > 0)
		{
			Pos = InLocalToWorld.TransformPosition(BeziersControlPoint.NextCtrlPointPos);
		}
		else
		{
			Pos = InLocalToWorld.TransformPosition(BeziersControlPoint.PrevCtrlPointPos);
		}
	}
	break;
	}

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
	HHitProxy* Default = GetPrimitiveSceneInfo()->DefaultDynamicHitProxy;
	//PDI->SetHitProxy(new HRuntimeSplinePointHitProxy(ComponentWeakPtr.Get()));
#endif

	PDI->DrawPoint(Pos, UseColor, DrawInfo.PointSize, DepthPriorityGroup);

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
	//PDI->SetHitProxy(nullptr);
#endif
}

void FRuntimeSplinePointSceneProxy::DrawDebugCollisions(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3CollisionInfo& CollisionInfo, uint8 DepthPriorityGroup) const
{
	if (!CollisionInfo.bDrawDebugCollision)
	{
		return;
	}
	FColor Color = CollisionInfo.DebugCollisionColor.ToFColor(true);
	float Thickness = CollisionInfo.DebugCollisionLineWidth;

#if DISABLE_COPY_IN_SPLINE_SCENE_PROXY
	EXEC_WITH_THREAD_MUTEX_LOCK(ComponentWeakPtr->RenderMuteX,
		if (!CollisionInfo.BodySetupPtr || !IsValid(*CollisionInfo.BodySetupPtr))
		{
			return;
		}
		const TArray<FKSphereElem> SphereElems = (*CollisionInfo.BodySetupPtr)->AggGeom.SphereElems;
	);
#else
	const TArray<FKSphereElem>& SphereElems = CollisionInfo.SphereElems;
#endif
	for (const FKSphereElem& Elem : SphereElems)
	{
		FTransform LocalTransform(Elem.Center);
		//FTransform LocalTransform(Elem.GetFinalScaled(FVector::OneVector, FTransform(CollisionInfo.CollisionLocalToWorld)).GetTransform());

		FMatrix ElemTM = LocalTransform.ToMatrixWithScale() * CollisionInfo.CollisionLocalToWorld;

		const FVector Origin = ElemTM.GetOrigin();
		const FVector XAxis = ElemTM.GetScaledAxis(EAxis::X);
		const FVector YAxis = ElemTM.GetScaledAxis(EAxis::Y);
		const FVector ZAxis = ElemTM.GetScaledAxis(EAxis::Z);

		DrawCircle(PDI, Origin, XAxis, YAxis, Color, Elem.Radius, DrawCollisionSides, DepthPriorityGroup, Thickness);
		DrawCircle(PDI, Origin, XAxis, ZAxis, Color, Elem.Radius, DrawCollisionSides, DepthPriorityGroup, Thickness);
		DrawCircle(PDI, Origin, YAxis, ZAxis, Color, Elem.Radius, DrawCollisionSides, DepthPriorityGroup, Thickness);
	}
}

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
EMouseCursor::Type HRuntimeSplinePointHitProxy::GetMouseCursor()
{
	const URuntimeSplinePointBaseComponent* SplinePoint = Cast<URuntimeSplinePointBaseComponent>(ComponentWeakPtr.Get());
	if (SplinePoint)
	{
		//TODO?
	}
	return HRuntimeSplinePrimitiveHitProxy::GetMouseCursor();
}
#endif
