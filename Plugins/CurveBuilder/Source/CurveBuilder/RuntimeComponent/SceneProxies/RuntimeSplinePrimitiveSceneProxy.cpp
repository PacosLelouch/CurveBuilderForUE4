// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplinePrimitiveSceneProxy.h"

const FVector FRuntimeSplinePrimitiveSceneProxy::CollisionScale3D = FVector::OneVector;
const int32 FRuntimeSplinePrimitiveSceneProxy::DrawCollisionSides = 16;

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
IMPLEMENT_HIT_PROXY(HRuntimeSplinePrimitiveHitProxy, HActor)
#endif

FPrimitiveViewRelevance FRuntimeSplinePrimitiveSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;

	Result.bDrawRelevance = IsComponentVaild() && IsShown(View); //bDrawDebug && !IsSelected() && IsShown(View) && View->Family->EngineShowFlags.Splines;

	if (IsDrawnInGame())
	{
		Result.bDrawRelevance = Result.bDrawRelevance
			&& (View->Family->EngineShowFlags.Lighting
				|| View->Family->EngineShowFlags.Wireframe
				|| View->Family->EngineShowFlags.Splines
				|| View->Family->EngineShowFlags.Collision);
	}
	else
	{
		Result.bDrawRelevance = Result.bDrawRelevance
			&& (View->Family->EngineShowFlags.Wireframe
				|| View->Family->EngineShowFlags.Splines
				|| View->Family->EngineShowFlags.Collision);
	}
	
	Result.bDynamicRelevance = true;
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
	return Result;
}

void FRuntimeSplinePrimitiveSceneProxy::DrawHalfCircle(FPrimitiveDrawInterface* PDI, const FVector& Base, const FVector& X, const FVector& Y, const FColor Color, float Radius, uint8 DepthPriorityGroup, float Thickness)
{
	float	AngleDelta = 2.0f * (float)PI / ((float)DrawCollisionSides);
	FVector	LastVertex = Base + X * Radius;

	for (int32 SideIndex = 0; SideIndex < (DrawCollisionSides >> 1); SideIndex++)
	{
		FVector	Vertex = Base + (X * FMath::Cos(AngleDelta * (SideIndex + 1)) + Y * FMath::Sin(AngleDelta * (SideIndex + 1))) * Radius;
		PDI->DrawLine(LastVertex, Vertex, Color, DepthPriorityGroup, Thickness);
		LastVertex = Vertex;
	}
}

#if ENABLE_CUSTOM_SPLINE_HIT_PROXY_RUNTIME
EMouseCursor::Type HRuntimeSplinePrimitiveHitProxy::GetMouseCursor()
{
	if (FRuntimeSplineCommandHelperBase* CommandHelper = GetCommandHelper())
	{
		//TODO
	}
	return EMouseCursor::Crosshairs;
}

FRuntimeSplineCommandHelperBase* HRuntimeSplinePrimitiveHitProxy::GetCommandHelper() const
{
	if (ComponentWeakPtr.IsValid())
	{
		if (ComponentWeakPtr.Get()->CommandHelper.IsValid())
		{
			return ComponentWeakPtr.Get()->CommandHelper.Get();
		}
	}
	return nullptr;
}
#endif
