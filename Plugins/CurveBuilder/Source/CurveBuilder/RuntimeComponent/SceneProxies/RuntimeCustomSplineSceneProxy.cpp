// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeCustomSplineSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"

static constexpr bool bDrawLineByCurveLength = false;

void FRuntimeCustomSplineSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeCustomSplineSceneProxy_GetDynamicMeshElements);

	//if (!IsValid(SplineComponent))
	//{
	//	return;
	//}

	//auto* SplineProxy = SplineComponent->GetSplineProxy();
	//if (!SplineProxy)
	//{
	//	return;
	//}

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

			DrawRuntimeSpline(
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

void FRuntimeCustomSplineSceneProxy::DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup)
{
	if (!View || !PDI)
	{
		return;
	}

#if DISABLE_COPY_IN_SPLINE_SCENE_PROXY
	if (!DrawInfo.SplineInternalWeakPtr.IsValid())
	{
		return;
	}
	const FSpatialSplineBase3& SplineInternal = *DrawInfo.SplineInternalWeakPtr.Pin().Get();
#else
	const FSpatialSplineBase3& SplineInternal = DrawInfo.SplineInternalRef.Get();
#endif

	TTuple<double, double> ParamRange = SplineInternal.GetParamRange();
	if (bDrawLineByCurveLength)
	{
		double Length = SplineInternal.GetLength(ParamRange.Get<1>());

		double SegNumDbl = FMath::CeilToDouble(Length / static_cast<double>(DrawInfo.SegLength));//FMath::CeilToDouble(Length * static_cast<double>(DrawInfo.NumSteps));
		int32 SegNum = FMath::RoundToInt(SegNumDbl);
		double StepLength = Length / SegNumDbl;

		double NextS = 0.;
		double T = ParamRange.Get<0>();
		FVector Start = LocalToWorld.TransformPosition(SplineInternal.GetPosition(T));
		for (int32 i = 0; i < SegNum; ++i)
		{
			NextS += StepLength;
			T = SplineInternal.GetParameterAtLength(NextS);

			FVector End = LocalToWorld.TransformPosition(SplineInternal.GetPosition(T));
			PDI->DrawLine(Start, End, DrawInfo.CurveColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
			Start = End;
		}
	}
	else
	{
		double DiffParam = ParamRange.Get<1>() - ParamRange.Get<0>();
		double SegNumDbl = FMath::CeilToDouble(DiffParam / static_cast<double>(DrawInfo.SegLength));//FMath::CeilToDouble(Length * static_cast<double>(DrawInfo.NumSteps));
		int32 SegNum = FMath::RoundToInt(SegNumDbl);
		double StepParam = DiffParam / SegNumDbl;

		double T = ParamRange.Get<0>();
		FVector Start = LocalToWorld.TransformPosition(SplineInternal.GetPosition(T));
		for (int32 i = 0; i < SegNum; ++i)
		{
			T += StepParam;

			FVector End = LocalToWorld.TransformPosition(SplineInternal.GetPosition(T));
			PDI->DrawLine(Start, End, DrawInfo.CurveColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
			Start = End;
		}
	}

	if (DrawInfo.bSelected)
	{
		switch (SplineInternal.GetType())
		{
		case ESplineType::ClampedBSpline:
		{
			const auto& BSpline = static_cast<const TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType&>(SplineInternal);
			TArray<FVector4> CtrlPoints;
			BSpline.GetCtrlPoints(CtrlPoints);

			if (CtrlPoints.Num() > 1)
			{
				FVector Start = LocalToWorld.TransformPosition(CtrlPoints[0]);
				//PDI->DrawPoint(Start, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
				for (int32 i = 1; i < CtrlPoints.Num(); ++i)
				{
					FVector End = LocalToWorld.TransformPosition(CtrlPoints[i]);
					PDI->DrawLine(Start, End, DrawInfo.CtrlSegColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
					Start = End;
					//PDI->DrawPoint(Start, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
				}
			}
		}
		break;
		case ESplineType::BezierString:
		{
			const auto& BezierString = static_cast<const TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType&>(SplineInternal);
			TArray<FVector4> CtrlPoints;
			TArray<FVector4> CtrlPointsPrev;
			TArray<FVector4> CtrlPointsNext;
			BezierString.GetCtrlPoints(CtrlPoints);
			BezierString.GetCtrlPointsPrev(CtrlPointsPrev);
			BezierString.GetCtrlPointsNext(CtrlPointsNext);

			if (CtrlPoints.Num() > 1)
			{
				for (int32 i = 0; i < CtrlPoints.Num(); ++i)
				{
					FVector Start = LocalToWorld.TransformPosition(CtrlPoints[i]);
					FVector Prev = LocalToWorld.TransformPosition(CtrlPointsPrev[i]);
					FVector Next = LocalToWorld.TransformPosition(CtrlPointsNext[i]);

					//PDI->DrawPoint(Start, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
					//PDI->DrawPoint(Prev, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
					//PDI->DrawPoint(Next, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);

					PDI->DrawLine(Start, Prev, DrawInfo.CtrlSegColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
					PDI->DrawLine(Start, Next, DrawInfo.CtrlSegColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
				}
			}
		}
		break;
		}
	}
}

void FRuntimeCustomSplineSceneProxy::DrawDebugCollisions(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3CollisionInfo& CollisionInfo, uint8 DepthPriorityGroup)
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
	const TArray<FKSphylElem>& SphylElems = (*CollisionInfo.BodySetupPtr)->AggGeom.SphylElems;
#else
	const TArray<FKSphylElem>& SphylElems = CollisionInfo.SphylElems;
#endif
	for (const FKSphylElem& Elem : SphylElems)
	{
		FTransform LocalTransform(Elem.Rotation, Elem.Center);
		//Elem.DrawElemWire(PDI, LocalTransform * FTransform(LocalToWorld), FVector::OneVector, SplineComponent->DebugCollisionColor.ToFColor(true));

		FMatrix ElemTM = LocalTransform.ToMatrixWithScale() * CollisionInfo.CollisionLocalToWorld;

		const FVector Origin = ElemTM.GetOrigin();
		const FVector XAxis = ElemTM.GetScaledAxis(EAxis::X);
		const FVector YAxis = ElemTM.GetScaledAxis(EAxis::Y);
		const FVector ZAxis = ElemTM.GetScaledAxis(EAxis::Z);
		const float ScaledHalfLength = Elem.GetScaledCylinderLength(CollisionScale3D) * .5f;
		const float ScaledRadius = Elem.GetScaledRadius(CollisionScale3D);

		// Draw top and bottom circles
		const FVector TopEnd = Origin + (ScaledHalfLength * ZAxis);
		const FVector BottomEnd = Origin - (ScaledHalfLength * ZAxis);

		DrawCircle(PDI, TopEnd, XAxis, YAxis, Color, ScaledRadius, DrawCollisionSides, DepthPriorityGroup, Thickness);
		DrawCircle(PDI, BottomEnd, XAxis, YAxis, Color, ScaledRadius, DrawCollisionSides, DepthPriorityGroup, Thickness);

		// Draw domed caps
		DrawHalfCircle(PDI, TopEnd, YAxis, ZAxis, Color, ScaledRadius, DepthPriorityGroup, Thickness);
		DrawHalfCircle(PDI, TopEnd, XAxis, ZAxis, Color, ScaledRadius, DepthPriorityGroup, Thickness);

		const FVector NegZAxis = -ZAxis;

		DrawHalfCircle(PDI, BottomEnd, YAxis, NegZAxis, Color, ScaledRadius, DepthPriorityGroup, Thickness);
		DrawHalfCircle(PDI, BottomEnd, XAxis, NegZAxis, Color, ScaledRadius, DepthPriorityGroup, Thickness);

		// Draw connecty lines
		PDI->DrawLine(TopEnd + ScaledRadius*XAxis, BottomEnd + ScaledRadius*XAxis, Color, DepthPriorityGroup, Thickness);
		PDI->DrawLine(TopEnd - ScaledRadius*XAxis, BottomEnd - ScaledRadius*XAxis, Color, DepthPriorityGroup, Thickness);
		PDI->DrawLine(TopEnd + ScaledRadius*YAxis, BottomEnd + ScaledRadius*YAxis, Color, DepthPriorityGroup, Thickness);
		PDI->DrawLine(TopEnd - ScaledRadius*YAxis, BottomEnd - ScaledRadius*YAxis, Color, DepthPriorityGroup, Thickness);
	}
}