// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeSplinePointBaseComponent.h"
#include "SceneProxies/RuntimeSplinePointSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"

URuntimeSplinePointBaseComponent::URuntimeSplinePointBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	//: Super()
{
}

void URuntimeSplinePointBaseComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URuntimeSplinePointBaseComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	SplinePointProxy.Reset();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

FPrimitiveSceneProxy* URuntimeSplinePointBaseComponent::CreateSceneProxy()
{
	return new FRuntimeSplinePointSceneProxy(this);
}

FBoxSphereBounds URuntimeSplinePointBaseComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FTransform RealLocalToWorld = GetSplineLocalToComponentLocalTransform() * LocalToWorld;
	FBoxSphereBounds SB(EForceInit::ForceInitToZero);

	if (IsValid(ParentSpline))
	{
		auto* Spline = ParentSpline->GetSplineProxy();
		if (Spline && SplinePointProxy.IsValid())
		{
			TSharedPtr<FSpatialControlPoint3> SpPSharedPtr = SplinePointProxy.Pin();
			SB.SphereRadius = CollisionDiameter;
			SB.BoxExtent = FVector(CollisionDiameter * 0.5f);
			switch (Spline->GetType())
			{
			case ESplineType::ClampedBSpline:
			{
				auto* BSplinePointProxy = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FControlPointType*>(SpPSharedPtr.Get());
				SB.Origin = TVecLib<4>::Projection(BSplinePointProxy->Pos);
			}
				break;
			case ESplineType::BezierString:
			{
				auto* BeziersPointProxy = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FControlPointType*>(SpPSharedPtr.Get());
				if (TangentFlag == 0)
				{
					SB.Origin = TVecLib<4>::Projection(BeziersPointProxy->Pos);
				}
				else if (TangentFlag > 0)
				{
					SB.Origin = TVecLib<4>::Projection(BeziersPointProxy->NextCtrlPointPos);
				}
				else
				{
					SB.Origin = TVecLib<4>::Projection(BeziersPointProxy->PrevCtrlPointPos);
				}
			}
				break;
			}
		}

		//if (Spline && PointIndex >= 0 && PointIndex < Spline->GetCtrlPointNum())
		//{
		//	SB.SphereRadius = CollisionDiameter;
		//	SB.BoxExtent = FVector(CollisionDiameter * 0.5f);
		//	switch (Spline->GetType())
		//	{
		//	case ESplineType::ClampedBSpline:
		//	{
		//		auto* BSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType*>(Spline);
		//		TArray<FVector4> CtrlPoints;
		//		BSpline->GetCtrlPoints(CtrlPoints);
		//		SB.Origin = TVecLib<4>::Projection(CtrlPoints[PointIndex]);
		//	}
		//		break;
		//	case ESplineType::BezierString:
		//	{
		//		auto* Beziers = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType*>(Spline);
		//		TArray<FVector4> CtrlPoints;
		//		Beziers->GetCtrlPoints(CtrlPoints);
		//		SB.Origin = TVecLib<4>::Projection(CtrlPoints[PointIndex]);
		//	}
		//		break;
		//	}
		//}
	}

	FBoxSphereBounds SBT = SB.TransformBy(RealLocalToWorld);
	return SBT;
}

void URuntimeSplinePointBaseComponent::UpdateCollision()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeSplinePointBaseComponent_UpdateCollision);

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	CreateBodySetup();

	//FMatrix LocalToWorld = GetSplineLocalToWorldMatrix();
	//FMatrix SplineLocalToComponentLocal = GetSplineLocalToComponentLocalTransform().ToMatrixWithScale();
	FTransform WorldToSplineLocal = GetWorldToSplineLocalTransform();
	FTransform SplineLocalToComponentLocal = GetSplineLocalToComponentLocalTransform();

	// Make sure that the bounds are the latest ones.
	SplineLocalPosition = WorldToSplineLocal.TransformPosition(Bounds.Origin);
	FVector Origin = SplineLocalToComponentLocal.TransformPosition(SplineLocalPosition);

	// Fill in simple collision sphyl elements
	BodySetup->AggGeom.SphereElems.SetNum(1);
	BodySetup->AggGeom.SphereElems[0].Center = Origin;
	BodySetup->AggGeom.SphereElems[0].Radius = CollisionDiameter;

	// Also we want cooked data for this
	BodySetup->bHasCookedCollisionData = true;
	BodySetup->InvalidatePhysicsData();
	//BodySetup->CreatePhysicsMeshes();
	RecreatePhysicsState();
}

#if WITH_EDITOR
void URuntimeSplinePointBaseComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const static FName LocationName("RelativeLocation");
	//const static FName RotationName("RelativeRotation");
	//const static FName ScaleName("RelativeScale3D");

	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();

	bool bLocationChanged = (PropertyName == LocationName || MemberPropertyName == LocationName);
	//bool bRotationChanged = (PropertyName == RotationName || MemberPropertyName == RotationName);
	//bool bScaleChanged = (PropertyName == ScaleName || MemberPropertyName == ScaleName);

	//if (bLocationChanged || bRotationChanged || bScaleChanged)
	if (bLocationChanged)
	{
		MoveSplinePointInternal();
		OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeSplinePointBaseComponent, DrawPointSize)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeSplinePointBaseComponent, CtrlPointColor)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeSplinePointBaseComponent, SelectedCtrlPointColor))
	{
		MarkRenderStateDirty();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeSplinePointBaseComponent, CollisionDiameter))
	{
		UpdateBounds();
		UpdateCollision();
		if (bDrawDebugCollision)
		{
			MarkRenderStateDirty();
		}
	}
}
void URuntimeSplinePointBaseComponent::PostEditComponentMove(bool bFinished)
{
	//if (bFinished)
	{
		MoveSplinePointInternal();
		OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
	}
	Super::PostEditComponentMove(bFinished);
}
#endif

void URuntimeSplinePointBaseComponent::SetSelected(bool bValue)
{
	if (bSelected != bValue)
	{
		bSelected = bValue;
		MarkRenderStateDirty();
	}
}

void URuntimeSplinePointBaseComponent::MoveSplinePointInternal()
{
	if (IsValid(ParentGraph) && IsValid(ParentSpline) && SplinePointProxy.IsValid())
	{
		auto* Spline = ParentSpline->GetSplineProxy();
		TSharedPtr<FSpatialControlPoint3> SpPSharedPtr = SplinePointProxy.Pin();
		if (Spline)
		{
			FTransform ParentComponentLocalToSplineLocal = GetParentComponentToSplineLocalTransform();
			FVector TargetSplineLocalPosition = ParentComponentLocalToSplineLocal.TransformPosition(GetRelativeLocation());
			ParentGraph->SplineGraphProxy.AdjustCtrlPointPos(
				*SpPSharedPtr.Get(), TargetSplineLocalPosition, ParentSpline->GetSplineProxyWeakPtr(),
				1, TangentFlag, 0);
			SplineLocalPosition = TargetSplineLocalPosition;
			ParentSpline->OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
		}
	}
}
