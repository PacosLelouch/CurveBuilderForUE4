// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplinePointBaseComponent.h"
#include "RuntimeCustomSplineBaseComponent.h"
#include "SceneProxies/RuntimeSplinePointSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"

URuntimeSplinePointBaseComponent::URuntimeSplinePointBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	//: Super()
{
	if (ObjectInitializer.GetArchetype())
	{
		bIsDuplicated = true;
	}
}

void URuntimeSplinePointBaseComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URuntimeSplinePointBaseComponent::OnVisibilityChanged()
{
	Super::OnVisibilityChanged();
	//if (bVisible && (bVisible != ParentSpline->bCustomSelected))
	//{
	//	ParentSpline->SetCustomSelected(bVisible);
	//}
}

void URuntimeSplinePointBaseComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (bIsDuplicated)
	{
		USceneComponent* RealParent = GetAttachParent();
		if (IsValid(RealParent) && !RealParent->IsBeingDestroyed())
		{
			const TArray<USceneComponent*>& Siblings = GetAttachParent()->GetAttachChildren();
			for (USceneComponent* Sibling : Siblings)
			{
				URuntimeCustomSplineBaseComponent* SpComp = Cast<URuntimeCustomSplineBaseComponent>(Sibling);
				if (IsValid(SpComp))
				{
					auto* Spline = SpComp->GetSplineProxy();
					if (Spline)
					{

						TangentFlag = 0;
						ParentGraph = SpComp->ParentGraph;
						ParentSpline = SpComp;
						FTransform ParentComponentLocalToSplineLocal = SpComp->GetParentComponentToSplineLocalTransform();
						FVector SplineLocalPos = ParentComponentLocalToSplineLocal.TransformPosition(GetRelativeLocation());
						Spline->AddPointAtLast(SplineLocalPos);
						SplinePointProxy = Spline->GetLastCtrlPointStruct();
						ParentSpline->PointComponents.Add(this);

						if (Spline->GetType() == ESplineType::BezierString)
						{
							const TSharedRef<FSpatialControlPoint3>& CPRef = SplinePointProxy.Pin().ToSharedRef();
							URuntimeSplinePointBaseComponent* PrevPoint = SpComp->AddPointInternal(CPRef, -1);
							PrevPoint->UpdateComponentLocationBySpline();
							URuntimeSplinePointBaseComponent* NextPoint = SpComp->AddPointInternal(CPRef, 1);
							NextPoint->UpdateComponentLocationBySpline();
						}
					}
					break;
				}
			}
		}
	}

	if (SplinePointProxy.IsValid())
	{
		FTransform SplineLocalToParentComponentLocal = GetSplineLocalToParentComponentTransform();
		SetRelativeLocation(SplineLocalToParentComponentLocal.TransformPosition(SplinePointProxy.Pin().Get()->Pos));
	}

	if (IsValid(ParentSpline) && !ParentSpline->IsBeingDestroyed())
	{
		ParentSpline->UpdateTransformByCtrlPoint();
		ParentSpline->OnSplineUpdatedEvent();

		if (IsValid(ParentSpline->SelectedPoint) && !ParentSpline->SelectedPoint->IsBeingDestroyed())
		{
			ParentSpline->SelectedPoint->SetCustomSelected(false);
		}
		if (ParentSpline->bAutoSelectNewPoint)
		{
			SetCustomSelected(true);
		}
	}
	UpdateCollision();
}

void URuntimeSplinePointBaseComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(ParentSpline) && !ParentSpline->IsBeingDestroyed())
	{
		auto* Spline = ParentSpline->GetSplineProxy();
		if (Spline && SplinePointProxy.IsValid())
		{
			Spline->RemovePoint(*SplinePointProxy.Pin().Get());
			//TODO: For Beziers, still need to destroy adjacent points.
		}
		if (ParentSpline->PointComponents.Contains(this))
		{
			ParentSpline->PointComponents.Remove(this);
		}
	}
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

	FTransform WorldToSplineLocal = GetWorldToSplineLocalTransform();
	//FTransform SplineLocalToComponentLocal = GetSplineLocalToParentComponentTransform();

	// Make sure that the bounds are the latest ones.
	SplineLocalPosition = WorldToSplineLocal.TransformPosition(Bounds.Origin);
	FVector Origin = FVector::ZeroVector;//SplineLocalToComponentLocal.TransformPosition(SplineLocalPosition);

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

void URuntimeSplinePointBaseComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	//UpdateComponentLocationBySpline();
	//MoveSplinePointInternal(); // Stack Overflow
	if (IsValid(ParentSpline) && !ParentSpline->IsBeingDestroyed())
	{
		ParentSpline->OnUpdateTransform(UpdateTransformFlags, Teleport);
		ParentSpline->OnSplineUpdatedEvent();
	}
}

bool URuntimeSplinePointBaseComponent::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit, EMoveComponentFlags MoveFlags, ETeleportType Teleport)
{
	//FVector PrevRelativeLocation = GetRelativeLocation();
	bool bReturn = Super::MoveComponentImpl(Delta, NewRotation, bSweep, Hit, MoveFlags, Teleport);
	if (bReturn && !Delta.IsNearlyZero(1e-3f))
	{
		MoveSplinePointInternal();
	}
	//SplineLocalPosition = ConvertPosition(GetRelativeLocation(), ECustomSplineCoordinateType::ComponentLocal, ECustomSplineCoordinateType::SplineGraphLocal);
	//UpdateComponentLocationBySpline();
	//MoveTo_Deprecated(PrevRelativeLocation + Delta, ECustomSplineCoordinateType::ComponentLocal);
	return bReturn;
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
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeSplinePointBaseComponent, bCustomSelected))
	{
		bCustomSelected = !bCustomSelected;
		SetCustomSelected(!bCustomSelected);
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

void URuntimeSplinePointBaseComponent::SetCustomSelected(bool bValue)
{
	if (bCustomSelected != bValue)
	{
		bCustomSelected = bValue;
		if (IsValid(ParentSpline) && !ParentSpline->IsBeingDestroyed())
		{
			if (bValue)
			{
				if (IsValid(ParentSpline->SelectedPoint) && !ParentSpline->SelectedPoint->IsBeingDestroyed())
				{
					ParentSpline->SelectedPoint->SetCustomSelected(false);
				}
				ParentSpline->SelectedPoint = this;
			}
			else if (ParentSpline->SelectedPoint == this)
			{
				ParentSpline->SelectedPoint = nullptr;
			}
		}
		MarkRenderStateDirty();
	}
}

void URuntimeSplinePointBaseComponent::MoveTo_Deprecated(const FVector& Position, ECustomSplineCoordinateType CoordinateType)
{
	if (IsValid(ParentSpline) && !ParentSpline->IsBeingDestroyed())
	{
		if (IsValid(ParentGraph) && !ParentGraph->IsActorBeingDestroyed())
		{
			ParentGraph->MovePoint(ParentSpline, this, Position, CoordinateType);
		}
		else
		{
			FVector ComponentLocalPosition = ConvertPosition(Position, CoordinateType, ECustomSplineCoordinateType::ComponentLocal);
			SetRelativeLocation(ComponentLocalPosition);
			MoveSplinePointInternal();
			//ParentSpline->UpdateTransformByCtrlPoint();
		}
	}
}

void URuntimeSplinePointBaseComponent::UpdateComponentLocationBySpline()
{
	if (SplinePointProxy.IsValid())
	{
		const FSpatialControlPoint3& PointStruct = *SplinePointProxy.Pin().Get();
		if (IsValid(ParentSpline) && !ParentSpline->IsBeingDestroyed())
		{
			auto* Spline = ParentSpline->GetSplineProxy();
			if (Spline)
			{
				if (Spline->GetType() == ESplineType::BezierString)
				{
					const auto& BezierPointStruct = static_cast<const TSplineTraitByType<ESplineType::BezierString, 3, 3>::FControlPointType&>(PointStruct);
					FVector4 SplineLocalPosition4 = (TangentFlag == 0 ? BezierPointStruct.Pos :
						(TangentFlag > 0 ? BezierPointStruct.NextCtrlPointPos : BezierPointStruct.PrevCtrlPointPos));
					SetRelativeLocation(GetSplineLocalToParentComponentTransform().TransformPosition(SplineLocalPosition4));
					return;
				}
			}
		}

		SetRelativeLocation(GetSplineLocalToParentComponentTransform().TransformPosition(PointStruct.Pos));
	}
}

void URuntimeSplinePointBaseComponent::MoveSplinePointInternal()
{
	if (IsValid(ParentSpline) && !ParentSpline->IsBeingDestroyed() && SplinePointProxy.IsValid())
	{
		//auto* Spline = ParentSpline->GetSplineProxy();
		//TSharedPtr<FSpatialControlPoint3> SpPSharedPtr = SplinePointProxy.Pin();
		//if (Spline)
		//{
		//	FTransform ParentComponentLocalToSplineLocal = GetParentComponentToSplineLocalTransform();
		//	FVector TargetSplineLocalPosition = ParentComponentLocalToSplineLocal.TransformPosition(GetRelativeLocation());
		//	ParentGraph->SplineGraphProxy.AdjustCtrlPointPos(
		//		*SpPSharedPtr.Get(), TargetSplineLocalPosition, ParentSpline->GetSplineProxyWeakPtr(),
		//		1, TangentFlag, 0);
		//	SplineLocalPosition = TargetSplineLocalPosition;
		//	ParentSpline->OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
		//}
		if (IsValid(ParentGraph))
		{
			SplineLocalPosition = ParentGraph->MovePoint(ParentSpline, this, GetRelativeLocation(), ECustomSplineCoordinateType::ComponentLocal);
		}
		else
		{
			ParentSpline->GetSplineProxy()->AdjustCtrlPointPos(*SplinePointProxy.Pin().Get(), GetParentComponentToSplineLocalTransform().TransformPosition(GetRelativeLocation()), TangentFlag, 0);
		}
		ParentSpline->UpdateTransformByCtrlPoint();
		ParentSpline->OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
	}
}
