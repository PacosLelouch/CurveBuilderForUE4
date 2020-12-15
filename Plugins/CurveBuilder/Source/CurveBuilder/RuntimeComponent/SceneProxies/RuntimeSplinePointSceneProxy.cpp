// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplinePointSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"

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
				DrawInfo, LocalToWorldMat,
				CurrentDepthPriorityGroup);

			DrawDebugCollisions(
				PDI, View,
				CollisionInfo,
				CurrentDepthPriorityGroup);

			DrawBoundsIfNecessary(PDI, ViewFamily);
		}
	}
}

void FRuntimeSplinePointSceneProxy::DrawRuntimeSplinePoint(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup)
{
	if (!View || !PDI)
	{
		return;
	}

#if DISABLE_COPY_IN_SPLINE_SCENE_PROXY
	if (!DrawInfo.SplinePointInternalWeakPtr.IsValid())
	{
		return;
	}
	const FSpatialControlPoint3& SplinePointInternal = *DrawInfo.SplinePointInternalWeakPtr.Pin().Get();
#else
	const FSpatialControlPoint3& SplinePointInternal = DrawInfo.SplinePointInternalRef.Get();
#endif

	FLinearColor UseColor = DrawInfo.bSelected ? DrawInfo.SelectedCtrlPointColor : DrawInfo.CtrlPointColor;

	switch (DrawInfo.SpType)
	{
	case ESplineType::ClampedBSpline:
	{
		const auto& BSplineControlPoint = static_cast<const TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FControlPointType&>(SplinePointInternal);
		FVector Pos = LocalToWorld.TransformPosition(BSplineControlPoint.Pos);
		PDI->DrawPoint(Pos, UseColor, DrawInfo.PointSize, DepthPriorityGroup);
	}
	break;
	case ESplineType::BezierString:
	{
		const auto& BeziersControlPoint = static_cast<const TSplineTraitByType<ESplineType::BezierString, 3, 3>::FControlPointType&>(SplinePointInternal);
		FVector Pos;
		if (DrawInfo.TangentFlag == 0)
		{
			Pos = LocalToWorld.TransformPosition(BeziersControlPoint.Pos);
		}
		else if (DrawInfo.TangentFlag > 0)
		{
			Pos = LocalToWorld.TransformPosition(BeziersControlPoint.NextCtrlPointPos);
		}
		else
		{
			Pos = LocalToWorld.TransformPosition(BeziersControlPoint.PrevCtrlPointPos);
		}
		PDI->DrawPoint(Pos, UseColor, DrawInfo.PointSize, DepthPriorityGroup);
	}
	break;
	}
}

void FRuntimeSplinePointSceneProxy::DrawDebugCollisions(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3CollisionInfo& CollisionInfo, uint8 DepthPriorityGroup)
{
	if (!CollisionInfo.bDrawDebugCollision)
	{
		return;
	}
	FColor Color = CollisionInfo.DebugCollisionColor.ToFColor(true);
	float Thickness = CollisionInfo.DebugCollisionLineWidth;

#if DISABLE_COPY_IN_SPLINE_SCENE_PROXY
	if (!CollisionInfo.BodySetupPtr || !IsValid(*CollisionInfo.BodySetupPtr))
	{
		return;
	}
	const TArray<FKSphereElem>& SphereElems = (*CollisionInfo.BodySetupPtr)->AggGeom.SphereElems;
#else
	const TArray<FKSphereElem>& SphereElems = CollisionInfo.SphereElems;
#endif
	for (const FKSphereElem& Elem : SphereElems)
	{
		FTransform LocalTransform(Elem.Center);

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