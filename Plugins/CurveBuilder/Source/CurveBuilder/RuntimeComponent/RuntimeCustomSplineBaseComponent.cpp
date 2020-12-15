// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeSplinePointBaseComponent.h"
#include "SceneProxies/RuntimeCustomSplineSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"

URuntimeCustomSplineBaseComponent::URuntimeCustomSplineBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void URuntimeCustomSplineBaseComponent::BeginPlay()
{
	Super::BeginPlay(); 
}

void URuntimeCustomSplineBaseComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	for (URuntimeSplinePointBaseComponent* PointComp : PointComponents)
	{
		if (IsValid(PointComp) && !PointComp->IsBeingDestroyed())
		{
			PointComp->DestroyComponent();
		}
	}
	PointComponents.Empty();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

FPrimitiveSceneProxy* URuntimeCustomSplineBaseComponent::CreateSceneProxy()
{
	return new FRuntimeCustomSplineSceneProxy(this);
}

//FMatrix URuntimeCustomSplineBaseComponent::GetRenderMatrix() const
//{
//	return GetSplineLocalToWorldMatrix();
//}

//void URuntimeCustomSplineBaseComponent::OnCreatePhysicsState()
//{
//#if true
//	Super::OnCreatePhysicsState();
//#else
//	USceneComponent::OnCreatePhysicsState();
//
//	// if we have a scene, we don't want to disable all physics and we have no bodyinstance already
//	//if (true)
//	if (!BodyInstance.IsValidBodyInstance())
//	{
//		//UE_LOG(LogPrimitiveComponent, Warning, TEXT("Creating Physics State (%s : %s)"), *GetNameSafe(GetOuter()),  *GetName());
//
//		UBodySetup* UseBodySetup = GetBodySetup();
//		if (UseBodySetup)
//		{
//			// Create new BodyInstance at given location.
//			FTransform BodyTransform = GetComponentTransform();//GetSplineLocalToWorldTransform();
//
//			// Here we make sure we don't have zero scale. This still results in a body being made and placed in
//			// world (very small) but is consistent with a body scaled to zero.
//			const FVector BodyScale = BodyTransform.GetScale3D();
//			if (BodyScale.IsNearlyZero())
//			{
//				BodyTransform.SetScale3D(FVector(KINDA_SMALL_NUMBER));
//			}
//#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
//			if ((BodyInstance.GetCollisionEnabled() != ECollisionEnabled::NoCollision) && (FMath::IsNearlyZero(BodyScale.X) || FMath::IsNearlyZero(BodyScale.Y) || FMath::IsNearlyZero(BodyScale.Z)))
//			{
//				UE_LOG(LogPhysics, Warning, TEXT("Scale for %s has a component set to zero, which will result in a bad body instance. Scale:%s"), *GetPathNameSafe(this), *BodyScale.ToString());
//			}
//#endif
//
//			// Create the body.
//			BodyInstance.InitBody(UseBodySetup, BodyTransform, this, GetWorld()->GetPhysicsScene());
//#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
//			SendRenderDebugPhysics();
//#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
//
//#if WITH_EDITOR
//			// Make sure we have a valid body instance here. As we do not keep BIs with no collision shapes at all,
//			// we don't want to create cloth collision in these cases
//			if (BodyInstance.IsValidBodyInstance())
//			{
//				const float RealMass = BodyInstance.GetBodyMass();
//				const float CalcedMass = BodySetup->CalculateMass(this);
//				float MassDifference = RealMass - CalcedMass;
//				if (RealMass > 1.0f && FMath::Abs(MassDifference) > 0.1f)
//				{
//					UE_LOG(LogPhysics, Log, TEXT("Calculated mass differs from real mass for %s:%s. Mass: %f  CalculatedMass: %f"),
//						GetOwner() != NULL ? *GetOwner()->GetName() : TEXT("NoActor"),
//						*GetName(), RealMass, CalcedMass);
//				}
//			}
//#endif // WITH_EDITOR
//		}
//	}
//#endif
//}

//void URuntimeCustomSplineBaseComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
//{
//	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
//	UpdateBounds();
//	UpdateCollision();
//	MarkRenderTransformDirty();  // Need to send new bounds to render thread
//}

FBoxSphereBounds URuntimeCustomSplineBaseComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	//FTransform RealLocalToWorld = GetSplineLocalToWorldTransform() * GetComponentTransform().Inverse() * LocalToWorld;
	FTransform RealLocalToWorld = GetSplineLocalToComponentLocalTransform() * LocalToWorld;
	
	FBox Box(EForceInit::ForceInitToZero);
	auto* Spline = GetSplineProxy();
	if (Spline)
	{
		switch (Spline->GetType())
		{
		case ESplineType::ClampedBSpline:
		{
			TArray<FVector4> CPs;
			auto* BSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline>::FSplineType*>(Spline);
			BSpline->GetCtrlPoints(CPs);
			for (const FVector4& V : CPs)
			{
				Box += TVecLib<4>::Projection(V);
			}
		}
			break;

		case ESplineType::BezierString:
		{
			TArray<FVector4> CPs, NextCPs, PrevCPs;
			auto* Beziers = static_cast<TSplineTraitByType<ESplineType::BezierString>::FSplineType*>(Spline);
			Beziers->GetCtrlPoints(CPs);
			Beziers->GetCtrlPointsNext(NextCPs);
			Beziers->GetCtrlPointsPrev(PrevCPs);
			for (const FVector4& V : CPs)
			{
				Box += TVecLib<4>::Projection(V);
			}
			for (const FVector4& V : NextCPs)
			{
				Box += TVecLib<4>::Projection(V);
			}
			for (const FVector4& V : PrevCPs)
			{
				Box += TVecLib<4>::Projection(V);
			}
		}
			break;
		}
	}

	FBoxSphereBounds SB(Box.ExpandBy(this->CollisionSegWidth));
	FBoxSphereBounds SBT = SB.TransformBy(RealLocalToWorld);
	return SBT;
}

//void URuntimeCustomSplineBaseComponent::CreateBodySetup()
//{
//	if (!IsValid(BodySetup))
//	{
//		BodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
//		BodySetup->BodySetupGuid = FGuid::NewGuid();
//
//		BodySetup->BuildScale3D = FVector::OneVector;
//		BodySetup->bGenerateMirroredCollision = false;
//		BodySetup->bDoubleSidedGeometry = false;
//		BodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;//bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;
//	}
//}

void URuntimeCustomSplineBaseComponent::UpdateCollision()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeCustomSplineBaseComponent_UpdateCollision);

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	auto* Spline = GetSplineProxy();

	if (!Spline)
	{
		return;
	}

	CreateBodySetup();

	TTuple<double, double> ParamRange = Spline->GetParamRange();

	double ParamDiff = ParamRange.Get<1>() - ParamRange.Get<0>();
	double SegNumDbl = FMath::CeilToDouble(ParamDiff / CollisionSegLength);
	int32 SegNum = FMath::FloorToInt(SegNumDbl);
	double StepLength = ParamDiff / SegNumDbl;

	//FMatrix LocalToWorld = GetSplineLocalToWorldMatrix();
	FMatrix SplineLocalToComponentLocal = GetSplineLocalToComponentLocalTransform().ToMatrixWithScale();
	double T = ParamRange.Get<0>();
	FVector Start = SplineLocalToComponentLocal.TransformPosition(Spline->GetPosition(T));

	// Fill in simple collision sphyl elements
	BodySetup->AggGeom.SphylElems.Empty(SegNum);
	for (int32 i = 0; i < SegNum; ++i)
	{
		T += StepLength;
		FVector End = SplineLocalToComponentLocal.TransformPosition(Spline->GetPosition(T));
		FVector SphylUpTangent = End - Start;
		FVector SphylUpDirection = SphylUpTangent.GetSafeNormal();
		//FVector SphylDirection = (FVector::UpVector ^ SphylUpDirection).GetSafeNormal();
		//if (SphylDirection.IsNearlyZero())
		//{
		//	SphylDirection = FVector::ForwardVector;
		//}
		FKSphylElem SegElem(CollisionSegWidth, SphylUpDirection | SphylUpTangent);
		SegElem.Center = (Start + End) * 0.5f;
		//FRotator DirRotator = SphylUpDirection.ToOrientationRotator();
		//SegElem.Rotation = (LocalToWorld.ToQuat() * DirRotator.Quaternion()).Rotator();
		//SegElem.Rotation = (LocalToWorld.ToQuat() * SphylDirection.ToOrientationRotator().Quaternion()).Rotator();
		//SegElem.Rotation = (SplineLocalToParentComponentLocal.Rotator().Quaternion() * FRotator(-90.f, 0.f, 0.f).Quaternion()).Rotator();
		SegElem.Rotation = (SphylUpDirection.ToOrientationQuat() * FRotator(-90.f, 0.f, 0.f).Quaternion()).Rotator();
		BodySetup->AggGeom.SphylElems.Add(SegElem);
		Start = End;
	}
	
	// Also we want cooked data for this
	BodySetup->bHasCookedCollisionData = true;
	BodySetup->InvalidatePhysicsData();
	//BodySetup->CreatePhysicsMeshes();
	RecreatePhysicsState();
}

//void URuntimeCustomSplineBaseComponent::SetDrawDebugCollision(bool bValue)
//{
//	if (bDrawDebugCollision != bValue)
//	{
//		bDrawDebugCollision = bValue;
//		UpdateCollision();
//	}
//}

#if WITH_EDITOR
void URuntimeCustomSplineBaseComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{

	const static FName LocationName("RelativeLocation");
	const static FName RotationName("RelativeRotation");
	const static FName ScaleName("RelativeScale3D");

	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();

	bool bLocationChanged = (PropertyName == LocationName || MemberPropertyName == LocationName);
	bool bRotationChanged = (PropertyName == RotationName || MemberPropertyName == RotationName);
	bool bScaleChanged = (PropertyName == ScaleName || MemberPropertyName == ScaleName);

	if (bLocationChanged || bRotationChanged || bScaleChanged)
	{
		UpdateTransformByCtrlPoint();
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, DrawSegLength)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, DrawThickness)
		//|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, DrawPointSize)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CurveColor)
		//|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CtrlPointColor)
		//|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, SelectedCtrlPointColor)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CtrlSegColor)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, DebugCollisionLineWidth)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, bDrawDebugCollision))
	{
		MarkRenderStateDirty();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CollisionSegLength)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CollisionSegWidth))
	{
		UpdateBounds();
		UpdateCollision();
		if (bDrawDebugCollision)
		{
			MarkRenderStateDirty();
		}
	}
}

void URuntimeCustomSplineBaseComponent::PostEditComponentMove(bool bFinished)
{
	//if (bFinished)
	{
		UpdateTransformByCtrlPoint();
	}
	Super::PostEditComponentMove(bFinished);
}
#endif

void URuntimeCustomSplineBaseComponent::SetSelected(bool bValue)
{
	if (bSelected != bValue)
	{
		bSelected = bValue;
		for (URuntimeSplinePointBaseComponent* PointComp : PointComponents)
		{
			PointComp->SetVisibility(bValue);
			PointComp->SetCollisionEnabled(bValue ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
			if (bValue)
			{
				PointComp->UpdateBounds();
				PointComp->UpdateCollision();
			}
		}
		MarkRenderStateDirty();
	}
}

void URuntimeCustomSplineBaseComponent::Reverse()
{
	auto* Spline = GetSplineProxy();
	if (!Spline)
	{
		return;
	}
	TArray<TWeakPtr<FSpatialControlPoint3>> CPs;
	TArray<FSpatialControlPoint3*> CPsRaw;
	TMap<FSpatialControlPoint3* , URuntimeSplinePointBaseComponent*> CPMap;
	CPMap.Reserve(Spline->GetCtrlPointNum());
	Spline->GetCtrlPointStructs(CPs);
	CPsRaw.Reserve(CPs.Num());
	for (const auto& Ptr : CPs)
	{
		CPsRaw.Add(Ptr.Pin().Get());
	}
	for (URuntimeSplinePointBaseComponent* CPComp : PointComponents)
	{
		if(IsValid(CPComp) && !CPComp->IsBeingDestroyed() && CPComp->SplinePointProxy.IsValid())
		CPMap.Add(CPComp->SplinePointProxy.Pin().Get(), CPComp);
	}

	Spline->Reverse();

	TArray<TWeakPtr<FSpatialControlPoint3>> NewCPs;
	Spline->GetCtrlPointStructs(NewCPs);
	for (int32 i = 0; i < NewCPs.Num(); ++i)
	{
		FSpatialControlPoint3* OldCP = CPsRaw.Last(i);
		URuntimeSplinePointBaseComponent* CPComp = CPMap[OldCP];
		CPComp->SplinePointProxy = NewCPs[i];
		CPComp->UpdateBounds();
		CPComp->UpdateCollision();
		CPComp->MarkRenderStateDirty();
	}

	UpdateBounds();
	UpdateCollision();
	MarkRenderStateDirty();
}

TWeakPtr<FSpatialSplineGraph3::FSplineType> URuntimeCustomSplineBaseComponent::GetSplineProxyWeakPtr() const
{
	if (SplineBaseWrapperProxy.Get() && SplineBaseWrapperProxy.Get()->Spline.IsValid())
	{
		return TWeakPtr<FSpatialSplineGraph3::FSplineType>(SplineBaseWrapperProxy.Get()->Spline);
	}
	return TWeakPtr<FSpatialSplineGraph3::FSplineType>(nullptr);
}

FSpatialSplineGraph3::FSplineType* URuntimeCustomSplineBaseComponent::GetSplineProxy() const
{
	if (SplineBaseWrapperProxy.IsValid() && SplineBaseWrapperProxy.Get()->Spline.IsValid())
	{
		return SplineBaseWrapperProxy.Get()->Spline.Get();
	}
	return nullptr;

	//if (SplineBaseWrapperProxy && SplineBaseWrapperProxy->Spline)
	//{
	//	switch (SplineBaseWrapperProxy->Spline->GetType())
	//	{
	//	case ESplineType::ClampedBSpline:
	//		return GetSplineProxyInternal<TSplineTraitByType<ESplineType::ClampedBSpline>::FSplineType>();
	//	case ESplineType::BezierString:
	//		return GetSplineProxyInternal<TSplineTraitByType<ESplineType::BezierString>::FSplineType>();
	//	}
	//}
	//return GetSplineProxyInternal<TSplineTraitByType<ESplineType::Unknown>::FSplineType>();
}

void URuntimeCustomSplineBaseComponent::AddPointInternal(const TSharedRef<FSpatialControlPoint3>& PointRef, int32 TangentFlag)
{
	auto* Spline = GetSplineProxy();
	if (Spline && IsValid(ParentGraph))
	{
		//URuntimeSplinePointBaseComponent* NewPoint = PointComponents.Add_GetRef(NewObject<URuntimeSplinePointBaseComponent>());
		//NewPoint->PointIndex = PointComponents.Num() - 1;
		USceneComponent* RealParent = GetAttachParent();
		FTransform SplineLocalToParentComponentLocal = GetSplineLocalToParentComponentTransform();

		URuntimeSplinePointBaseComponent* NewPoint = NewObject<URuntimeSplinePointBaseComponent>(RealParent);
		PointComponents.Add(NewPoint);
		NewPoint->SplinePointProxy = TWeakPtr<FSpatialControlPoint3>(PointRef);
		NewPoint->TangentFlag = TangentFlag;
		NewPoint->ParentSpline = this;
		NewPoint->ParentGraph = ParentGraph;
		NewPoint->AttachToComponent(RealParent, FAttachmentTransformRules::KeepRelativeTransform);
		this->GetOwner()->AddInstanceComponent(NewPoint);
		NewPoint->SetRelativeLocation(SplineLocalToParentComponentLocal.TransformPosition(PointRef.Get().Pos));
		NewPoint->RegisterComponent();

		UpdateTransformByCtrlPoint();
	}
}

void URuntimeCustomSplineBaseComponent::UpdateTransformByCtrlPoint()
{
	auto* Spline = GetSplineProxy();
	if (Spline && Spline->GetCtrlPointNum() > 0)
	{
		TTuple<double, double> ParamRange = Spline->GetParamRange();
		FVector SplineLocalPosition = Spline->GetPosition(ParamRange.Get<0>());
		FVector ComponentLocalPosition = GetSplineLocalToParentComponentTransform().TransformPosition(SplineLocalPosition);
		if ((ComponentLocalPosition - GetRelativeLocation()).IsNearlyZero())
		{
			OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
		}
		else
		{
			SetRelativeTransform(FTransform(FRotator::ZeroRotator, ComponentLocalPosition));
		}
	}
	//UpdateBounds();
	//UpdateCollision();
	//MarkRenderTransformDirty();
}