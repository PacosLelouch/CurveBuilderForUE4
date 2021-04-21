// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeSplinePointBaseComponent.h"
#include "SceneProxies/RuntimeCustomSplineSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "SerializeUtils/SplineSerialize.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/Multibox/MultiboxBuilder.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "RuntimeSplineCommandHelper"

static const auto LengthFactor = 100.f, InvLengthFactor = 1.f / 100.f;

URuntimeCustomSplineBaseComponent::URuntimeCustomSplineBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CustomSplinePointClass = URuntimeSplinePointBaseComponent::StaticClass();
	bLastCreateCollisionByCurveLength = bCreateCollisionByCurveLength;
	CollisionSegLength *= bCreateCollisionByCurveLength ? LengthFactor : 1.f;
}

void URuntimeCustomSplineBaseComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URuntimeCustomSplineBaseComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	InitializeCommandHelper();
}

void URuntimeCustomSplineBaseComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	ClearSpline();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void URuntimeCustomSplineBaseComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();
	AActor* CurrentAttachedActor = GetAttachmentRootActor();
	if (PreviousAttachedActor != CurrentAttachedActor)
	{
		PreviousAttachedActor = CurrentAttachedActor;
		OnActorAttachedEvent(CurrentAttachedActor);
	}
}

bool URuntimeCustomSplineBaseComponent::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit, EMoveComponentFlags MoveFlags, ETeleportType Teleport)
{
	bool bReturnValue = Super::MoveComponentImpl(Delta, NewRotation, bSweep, Hit, MoveFlags, Teleport);
	if (bReturnValue && !Delta.IsNearlyZero(1e-3))
	{
		UpdateTransformByCtrlPoint();
	}
	return bReturnValue;
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
			auto* BSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType*>(Spline);
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
			auto* Beziers = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType*>(Spline);
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

	TArray<double> Parameters;
	int32 SegNum = SampleParameters(Parameters, *Spline, CollisionSegLength, bCreateCollisionByCurveLength);

	//FMatrix LocalToWorld = GetSplineLocalToWorldMatrix();
	FMatrix SplineLocalToComponentLocal = GetSplineLocalToComponentLocalTransform().ToMatrixWithScale();
	double T = Parameters[0];
	FVector Start = SplineLocalToComponentLocal.TransformPosition(Spline->GetPosition(T));

	// Fill in simple collision sphyl elements
	BodySetup->AggGeom.SphylElems.Empty(Parameters.Num() - 1);
	for (int32 i = 0; i < SegNum; ++i)
	{
		T = Parameters[i + 1];
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

void URuntimeCustomSplineBaseComponent::InitializeCommandHelper()
{
	CommandHelper = MakeShareable(new FRuntimeSplineCommandHelper(this));
	CommandHelper.Get()->MapActions();
}

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
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CollisionSegWidth)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, bCreateCollisionByCurveLength))
	{
		if (bLastCreateCollisionByCurveLength != bCreateCollisionByCurveLength)
		{
			CollisionSegLength *= bCreateCollisionByCurveLength ? LengthFactor : InvLengthFactor;
			bLastCreateCollisionByCurveLength = bCreateCollisionByCurveLength;
		}
		UpdateBounds();
		UpdateCollision();
		if (bDrawDebugCollision)
		{
			MarkRenderStateDirty();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, bCustomSelected))
	{
		bCustomSelected = !bCustomSelected;
		SetCustomSelected(!bCustomSelected);
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

void URuntimeCustomSplineBaseComponent::Serialize(FArchive& Ar)
{
	//TMap<URuntimeSplinePointBaseComponent*, int32> PointIds;
	if (SplineSerializeUtils::ShouldSaveArchive(Ar))
	{
		Super::Serialize(Ar);
		if (ParentGraph == nullptr)
		{
			//PointIds.Reserve(PointComponents.Num());
			//for (URuntimeSplinePointBaseComponent* Point : PointComponents)
			//{
			//	PointIds.Add(Point, PointIds.Num());
			//}
			uint8 bValidSerialize;
			if (SplineBaseWrapperProxy.IsValid())
			{
				bValidSerialize = true;
				SplineSerializeUtils::SerializeSplineWrapper(Ar, SplineBaseWrapperProxy.Get(), bValidSerialize);
			}
			else
			{
				bValidSerialize = false;
				SplineSerializeUtils::SerializeSplineWrapper(Ar, nullptr, bValidSerialize);
			}
		}
	}
	else //if (SplineSerializeUtils::ShouldLoadArchive(Ar))
	{
		Super::Serialize(Ar);
		if (ParentGraph == nullptr)
		{
			uint8 bValidSerialize;
			SplineBaseWrapperProxy = MakeShareable(new FSpatialSplineGraph3::FSplineWrapper());
			SplineSerializeUtils::SerializeSplineWrapper(Ar, SplineBaseWrapperProxy.Get(), bValidSerialize);
			if (bValidSerialize && SplineBaseWrapperProxy.IsValid())
			{
				auto* Wrapper = SplineBaseWrapperProxy.Get();
				if (Wrapper->Spline.IsValid())
				{
					SplineSerializeUtils::DistributeSplinePoints(PointComponents, Wrapper->Spline.Get());
				}
			}
		}
	}
}

void URuntimeCustomSplineBaseComponent::OnSplineUpdatedEvent()
{
	OnSplineUpdateHandle.Broadcast(this);
}

void URuntimeCustomSplineBaseComponent::OnActorAttachedEvent_Implementation(AActor* AttachedActor)
{
	if (IsValid(AttachedActor) && !AttachedActor->IsActorBeingDestroyed())
	{
		OnActorAttachedHandle.Broadcast(AttachedActor);
	}
}

void URuntimeCustomSplineBaseComponent::SetCustomSelected(bool bValue)
{
	if (bCustomSelected != bValue)
	{
		bCustomSelected = bValue;
		if (IsValid(ParentGraph) && !ParentGraph->IsActorBeingDestroyed())
		{
			if (bValue)
			{
				if (!ParentGraph->SelectedSplines.Contains(this))
				{
					ParentGraph->SelectedSplines.Add(this);
				}
			}
			else
			{
				ParentGraph->SelectedSplines.RemoveSingle(this);
			}
		}
		
		for (URuntimeSplinePointBaseComponent* PointComp : PointComponents)
		{
			PointComp->SetVisibility(bValue);
			PointComp->SetCollisionEnabled(bValue ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
			if (bValue)
			{
				PointComp->UpdateBounds();
				PointComp->UpdateCollision();
			}
			else
			{
				PointComp->SetCustomSelected(false);
			}
		}
		MarkRenderStateDirty();
	}
}

ERuntimeSplineType URuntimeCustomSplineBaseComponent::GetSplineType() const
{
	const auto* Spline = GetSplineProxy();
	if (Spline)
	{
		return ARuntimeSplineGraph::GetExternalSplineType(Spline->GetType());
	}
	return ERuntimeSplineType::Unknown;
}

void URuntimeCustomSplineBaseComponent::GetParametricCubicPolynomialForms(
	TArray<FVector>& OutParamPoly3Forms,
	ECustomSplineCoordinateType TargetCoordinateType)
{
	auto* Spline = GetSplineProxy();
	if (Spline)
	{
		TArray<TArray<FVector4> > GroupedParamPoly3Forms;
		Spline->ToPolynomialForm(GroupedParamPoly3Forms);
		int32 GroupNum = GroupedParamPoly3Forms.Num();
		OutParamPoly3Forms.Empty((GroupNum > 0) ? GroupNum * GroupedParamPoly3Forms[0].Num() : 0);
		for (const TArray<FVector4>& ParamPoly3 : GroupedParamPoly3Forms)
		{
			for (int32 i = 0; i < ParamPoly3.Num(); ++i)
			{
				FVector SplineLocalPos = TVecLib<4>::Projection(ParamPoly3[i]);
				OutParamPoly3Forms.Add(ConvertPosition(SplineLocalPos, ECustomSplineCoordinateType::SplineGraphLocal, TargetCoordinateType));
			}
		}
	}
}

void URuntimeCustomSplineBaseComponent::GetCubicBezierForms(
	TArray<FVector>& OutBezierForms,
	ECustomSplineCoordinateType TargetCoordinateType)
{
	auto* Spline = GetSplineProxy();
	if (Spline)
	{
		TArray<TBezierCurve<3, 3> > BezierCurves;
		Spline->ToBezierCurves(BezierCurves);
		int32 CurveNum = BezierCurves.Num();
		OutBezierForms.Empty(CurveNum > 0 ? CurveNum * BezierCurves[0].CurveOrder() : 0);
		for (const TBezierCurve<3, 3>& Curve : BezierCurves)
		{
			for (int32 i = 0; i < Curve.CurveOrder(); ++i)
			{
				FVector SplineLocalPos = Curve.GetPoint(i);
				OutBezierForms.Add(ConvertPosition(SplineLocalPos, ECustomSplineCoordinateType::SplineGraphLocal, TargetCoordinateType));
			}
		}
	}
}

void URuntimeCustomSplineBaseComponent::GetHermiteForms(
	TArray<FVector>& OutPositions, TArray<FVector>& OutArriveTangents, TArray<FVector>& OutLeaveTangents,
	ECustomSplineCoordinateType TargetCoordinateType)
{
	auto* Spline = GetSplineProxy();
	if (Spline)
	{
		TArray<TBezierCurve<3, 3> > BezierCurves;
		Spline->ToBezierCurves(BezierCurves);
		int32 CurveNum = BezierCurves.Num();
		OutPositions.Empty(CurveNum + 1);
		OutArriveTangents.Empty(CurveNum + 1);
		OutLeaveTangents.Empty(CurveNum + 1);
		if (CurveNum > 0)
		{
			const TBezierCurve<3, 3>& FirstCurve = BezierCurves[0];
			double Mult = static_cast<double>(FirstCurve.CurveDegree());
			OutArriveTangents.Add((
				ConvertPosition(FirstCurve.GetPoint(1), ECustomSplineCoordinateType::SplineGraphLocal, TargetCoordinateType) -
				ConvertPosition(FirstCurve.GetPoint(0), ECustomSplineCoordinateType::SplineGraphLocal, TargetCoordinateType))
				* Mult);
			for (int32 i = 0; i < CurveNum; ++i)
			{
				const TBezierCurve<3, 3>& Curve = BezierCurves[i];
				TArray<FVector> Positions;
				Positions.Reserve(Curve.CurveOrder());
				for (int32 j = 0; j < Curve.CurveOrder(); ++j)
				{
					Positions.Add(ConvertPosition(Curve.GetPoint(j), ECustomSplineCoordinateType::SplineGraphLocal, TargetCoordinateType));
				}
				OutPositions.Add(Positions[0]);
				OutArriveTangents.Add((Positions[Curve.CurveDegree()] - Positions[Curve.CurveDegree() - 1]) * Mult);
				OutLeaveTangents.Add((Positions[1] - Positions[0]) * Mult);
			}
			const TBezierCurve<3, 3>& LastCurve = BezierCurves.Last();
			FVector Last0 = ConvertPosition(
				LastCurve.GetPoint(LastCurve.CurveDegree()),
				ECustomSplineCoordinateType::SplineGraphLocal, TargetCoordinateType);
			FVector Last1 = ConvertPosition(
				LastCurve.GetPoint(LastCurve.CurveDegree() - 1),
				ECustomSplineCoordinateType::SplineGraphLocal, TargetCoordinateType);
			OutPositions.Add(Last0);
			OutLeaveTangents.Add((Last0 - Last1) * Mult);
		}
	}
}

void URuntimeCustomSplineBaseComponent::Reverse()
{
	ReverseImpl();
}

void URuntimeCustomSplineBaseComponent::ClearSpline()
{
	if (IsValid(ParentGraph) && !ParentGraph->IsActorBeingDestroyed())
	{
		ParentGraph->RemoveSplineFromGraph(this);
		//URuntimeCustomSplineBaseComponent** CompPtr = ParentGraph->SplineComponentMap.Find(SplineBaseWrapperProxy);
		//if (CompPtr && *CompPtr == this)
		//{
		//	ParentGraph->SplineComponentMap.Remove(SplineBaseWrapperProxy);
		//}
	}
	SCOPE_MUTEX_LOCK(RenderMuteX);
	if (PointComponents.Num() > 0)
	{
		int32 Num = PointComponents.Num();
		for (auto It = PointComponents.CreateIterator(); It; ++It)
		{
			URuntimeSplinePointBaseComponent* PointComp = *It;
			if (IsValid(PointComp) && !PointComp->IsBeingDestroyed())
			{
				PointComp->DestroyComponent();
			}
		}
	}
	PointComponents.Empty();
	SelectedPoint = nullptr;
	
	if (SplineBaseWrapperProxy.IsValid())
	{
		SplineBaseWrapperProxy.Get()->Spline.Reset();
	}
}

URuntimeSplinePointBaseComponent* URuntimeCustomSplineBaseComponent::AddEndPoint(
	const FVector& Position,
	bool bAtLast, 
	ECustomSplineCoordinateType CoordinateType)
{
	auto* Spline = GetSplineProxy();
	if (Spline)
	{
		FVector SplineLocalPosition = ConvertPosition(Position, CoordinateType, ECustomSplineCoordinateType::SplineGraphLocal);
		TWeakPtr<FSpatialControlPoint3> ControlPointWeakPtr;
		if (bAtLast)
		{
			SCOPE_MUTEX_LOCK(RenderMuteX);
			Spline->AddPointAtLast(SplineLocalPosition);
			ControlPointWeakPtr = Spline->GetLastCtrlPointStruct();
		}
		else
		{
			SCOPE_MUTEX_LOCK(RenderMuteX);
			Spline->AddPointAtFirst(SplineLocalPosition);
			ControlPointWeakPtr = Spline->GetLastCtrlPointStruct();
		}
		if (ControlPointWeakPtr.IsValid())
		{
			TSharedRef<FSpatialControlPoint3> ControlPointRef = ControlPointWeakPtr.Pin().ToSharedRef();
			URuntimeSplinePointBaseComponent* CurrentPoint = AddPointInternal(ControlPointRef, 0);
			if (Spline->GetType() == ESplineType::BezierString)
			{
				// Need to make connection among three points?
				AddPointInternal(ControlPointRef, -1)->UpdateComponentLocationBySpline();
				AddPointInternal(ControlPointRef, 1)->UpdateComponentLocationBySpline();
			}
			
			return CurrentPoint;
		}
	}
	return nullptr;
}

URuntimeSplinePointBaseComponent* URuntimeCustomSplineBaseComponent::InsertPoint(const FVector& Position, bool& bSucceedReturn, ECustomSplineCoordinateType CoordinateType)
{
	auto* Spline = GetSplineProxy();
	if (Spline)
	{
		double Param = -1.; 
		FVector SplineLocalPosition = Position;
		switch (CoordinateType)
		{
		case ECustomSplineCoordinateType::ComponentLocal:
			SplineLocalPosition = GetParentComponentToSplineLocalTransform().TransformPosition(Position);
			break;
		case ECustomSplineCoordinateType::World:
			SplineLocalPosition = GetWorldToSplineLocalTransform().TransformPosition(Position);
			break;
		}
		
		if (Spline->FindParamByPosition(Param, SplineLocalPosition, FMath::Square(CollisionSegWidth)))
		{
			URuntimeSplinePointBaseComponent* NewPointComponent = nullptr;
			switch (Spline->GetType()) {
			case ESplineType::BezierString:
			{
				SCOPE_MUTEX_LOCK(RenderMuteX);
				auto* NewNode = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType*>(Spline)->AddPointWithParamWithoutChangingShape(Param);
				if (NewNode) {
					NewNode->GetValue().Get().Continuity = EEndPointContinuity::G1;
					NewPointComponent = AddPointInternal(NewNode->GetValue(), 0);
					AddPointInternal(NewNode->GetValue(), -1);
					AddPointInternal(NewNode->GetValue(), 1);
				}
			}
				break;
			case ESplineType::ClampedBSpline:
			{
				SCOPE_MUTEX_LOCK(RenderMuteX);
				auto* NewNode = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType*>(Spline)->AddPointWithParamWithoutChangingShape(Param);
				if (NewNode) {
					NewPointComponent = AddPointInternal(NewNode->GetValue(), 0);
				}
			}
				break;
			}
			if (Spline->GetType() != ESplineType::Unknown)
			{
				bSucceedReturn = (NewPointComponent != nullptr);
				if (bSucceedReturn)
				{
					UpdateControlPointsLocation();
				}
				return NewPointComponent;
			}
		}
	}

	bSucceedReturn = false;
	return nullptr;
}

void URuntimeCustomSplineBaseComponent::UpdateControlPointsLocation()
{
	for (URuntimeSplinePointBaseComponent* CPComp : PointComponents)
	{
		if (IsValid(CPComp) && !CPComp->IsBeingDestroyed())
		{
			CPComp->UpdateComponentLocationBySpline();
		}
	}
}

void URuntimeCustomSplineBaseComponent::ReverseImpl()
{
	auto* Spline = GetSplineProxy();
	if (!Spline)
	{
		return;
	}
	TArray<TWeakPtr<FSpatialControlPoint3>> CPs;
	TArray<FSpatialControlPoint3*> CPsRaw;
	TMap<FSpatialControlPoint3*, URuntimeSplinePointBaseComponent*> CPMap;
	CPMap.Reserve(Spline->GetCtrlPointNum());
	Spline->GetCtrlPointStructs(CPs);
	CPsRaw.Reserve(CPs.Num());
	for (const auto& Ptr : CPs)
	{
		CPsRaw.Add(Ptr.Pin().Get());
	}
	for (URuntimeSplinePointBaseComponent* CPComp : PointComponents)
	{
		if (IsValid(CPComp) && !CPComp->IsBeingDestroyed() && CPComp->SplinePointProxy.IsValid())
			CPMap.Add(CPComp->SplinePointProxy.Pin().Get(), CPComp);
	}

	EXEC_WITH_THREAD_MUTEX_LOCK(RenderMuteX,
		if (IsValid(ParentGraph) && !ParentGraph->IsActorBeingDestroyed())
		{
			ParentGraph->SplineGraphProxy.ReverseSpline(GetSplineProxyWeakPtr());
		}
		else
		{
			Spline->Reverse();
		}
	);

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
	//		return GetSplineProxyInternal<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType>();
	//	case ESplineType::BezierString:
	//		return GetSplineProxyInternal<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType>();
	//	}
	//}
	//return GetSplineProxyInternal<TSplineTraitByType<ESplineType::Unknown, 3, 3>::FSplineType>();
}

URuntimeSplinePointBaseComponent* URuntimeCustomSplineBaseComponent::AddPointInternal(const TSharedRef<FSpatialControlPoint3>& PointRef, int32 TangentFlag)
{
	auto* Spline = GetSplineProxy();
	if (Spline)
	{
		USceneComponent* RealParent = GetAttachParent();
		//URuntimeSplinePointBaseComponent* NewPoint = PointComponents.Add_GetRef(NewObject<URuntimeSplinePointBaseComponent>(RealParent, CustomSplinePointClass));
		//NewPoint->PointIndex = PointComponents.Num() - 1;
		if (IsValid(RealParent) && !RealParent->IsBeingDestroyed())
		{
			FTransform SplineLocalToParentComponentLocal = GetSplineLocalToParentComponentTransform();

			URuntimeSplinePointBaseComponent* NewPoint = nullptr;

			//FName ObjectName = FName(*(TEXT("URuntimeSplinePointBaseComponent_") + FString::FromInt(PointComponents.Num())));
			
			NewPoint = NewObject<URuntimeSplinePointBaseComponent>(RealParent, CustomSplinePointClass);
			NewPoint->Mobility = RealParent->Mobility;
			
			PointComponents.Add(NewPoint);
			NewPoint->SplinePointProxy = TWeakPtr<FSpatialControlPoint3>(PointRef);
			NewPoint->TangentFlag = TangentFlag;
			NewPoint->ParentSpline = this;
			NewPoint->ParentGraph = ParentGraph; // ParentGraph can be non-exist, which means that this spline is independent.
			NewPoint->AttachToComponent(RealParent, FAttachmentTransformRules::KeepRelativeTransform);
			AActor* Owner = GetOwner();
			if (IsValid(Owner))
			{
				Owner->AddInstanceComponent(NewPoint);
				NewPoint->RegisterComponent();
			}
			//NewPoint->SetRelativeLocation(SplineLocalToParentComponentLocal.TransformPosition(PointRef.Get().Pos)); // Move to Point OnComponentCreated
			NewPoint->SetVisibility(bCustomSelected);

			//UpdateTransformByCtrlPoint(); // Move to Point OnComponentCreated
			return NewPoint;
		}
	}
	return nullptr;
}

void URuntimeCustomSplineBaseComponent::UpdateTransformByCtrlPoint()
{
	auto* Spline = GetSplineProxy();
	if (IsValid(this) && !this->IsBeingDestroyed() && Spline && Spline->GetCtrlPointNum() > 0)
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

		//if (Spline->GetType() == ESplineType::BezierString)
		//{
		//	UpdateControlPointsLocation();
		//}
	}
	//UpdateBounds();
	//UpdateCollision();
	//MarkRenderTransformDirty();
}

int32 URuntimeCustomSplineBaseComponent::SampleParameters(TArray<double>& OutParameters, const FSpatialSplineBase3& SplineInternal, double SegLength, bool bByCurveLength, bool bAdjustKeyLength)
{
	TTuple<double, double> ParamRange = SplineInternal.GetParamRange();

	if (bByCurveLength)
	{
		double Length = SplineInternal.GetLength(ParamRange.Get<1>());

		double SegNumDbl = FMath::CeilToDouble(Length / SegLength);
		int32 SegNum = FMath::RoundToInt(SegNumDbl);
		double StepLength = Length / SegNumDbl;

		double T = ParamRange.Get<0>();
		double S = 0.;
		OutParameters.Empty(SegNum);
		OutParameters.Add(T);
		for (int32 i = 0; i < SegNum; ++i)
		{
			S += StepLength;
			T = SplineInternal.GetParameterAtLength(S);
			OutParameters.Add(T);
		}
	}
	else
	{
		double ParamDiff = ParamRange.Get<1>() - ParamRange.Get<0>();
		TArray<double> SegParams;
		if (bAdjustKeyLength)
		{
			SplineInternal.GetSegParams(SegParams);
		}
		if (SegParams.Num() <= 1 || !bAdjustKeyLength)
		{
			SegParams = { ParamRange.Get<0>(), ParamRange.Get<1>() };
		}

		double SegNumDbl = 0., StepLength = 0.;
		double T = SegParams[0];
		for (int32 Seg = 0; Seg + 1 < SegParams.Num(); ++Seg)
		{
			if (bAdjustKeyLength)
			{
				if (Seg == 0)
				{
					SegNumDbl = FMath::CeilToDouble(1. / SegLength);
					OutParameters.Empty((SegParams.Num() - 1) * SegNumDbl);
					OutParameters.Add(T);
				}
				double SegParamDiff = SegParams[Seg + 1] - SegParams[Seg];
				StepLength = SegParamDiff / SegNumDbl;
			}
			else
			{
				SegNumDbl = FMath::CeilToDouble(ParamDiff / SegLength);
				StepLength = ParamDiff / SegNumDbl;
				OutParameters.Empty((SegParams.Num() - 1) * SegNumDbl);
				OutParameters.Add(T);
			}
			int32 SegNum = FMath::RoundToInt(SegNumDbl);

			T = SegParams[Seg]; // To avoid error when the spline is very long.
			for (int32 i = 0; i < SegNum; ++i)
			{
				T += StepLength;
				OutParameters.Add(T);
			}
		}
	}

	return OutParameters.Num() - 1;
}

void FRuntimeSplineCommandHelper::CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	FRuntimeSplineCommandHelperBase::CapturedMouseMove(InViewport, InMouseX, InMouseY);
}

bool FRuntimeSplineCommandHelper::InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	bool bBaseReturnValue = FRuntimeSplineCommandHelperBase::InputKey(Viewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
	return bBaseReturnValue;
}

bool FRuntimeSplineCommandHelper::InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	bool bBaseReturnValue = FRuntimeSplineCommandHelperBase::InputAxis(Viewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
	return bBaseReturnValue;
}

void FRuntimeSplineCommandHelper::OnInsertControlPoint()
{
	auto* Component = ComponentWeakPtr.Get();
	bool bSucceedReturn = false;
	Component->InsertPoint(LastSnappedWorldPosition.GetValue(), bSucceedReturn, ECustomSplineCoordinateType::World);
}

bool FRuntimeSplineCommandHelper::CanInsertControlPoint() const
{
	if (!ComponentWeakPtr.IsValid() || !LastSnappedWorldPosition.IsSet())
	{
		return false;
	}
	if (ComponentWeakPtr->PointComponents.Num() < 2)
	{
		return false;
	}

	return true;
}

void FRuntimeSplineCommandHelper::OnAddNewSplineAdj(bool bAtLast)
{
	auto* Component = ComponentWeakPtr.Get();
	auto* Graph = Component->ParentGraph;
	bool bSucceedReturn = false;
	Graph->ExtendNewSplineWithContinuity(bSucceedReturn, Component, bAtLast);
}

bool FRuntimeSplineCommandHelper::CanAddNewSplineAdj() const
{
	if (!ComponentWeakPtr.IsValid() || !LastSnappedWorldPosition.IsSet())
	{
		return false;
	}
	if (!IsValid(ComponentWeakPtr->ParentGraph) || ComponentWeakPtr->ParentGraph->IsActorBeingDestroyed())
	{
		return false;
	}
	if (ComponentWeakPtr->PointComponents.Num() < 2)
	{
		return false;
	}
	return true;
}
//
//void FRuntimeSplineCommandHelper::OnAddNewSplineAtStart()
//{
//	auto* Component = ComponentWeakPtr.Get();
//	auto* Graph = Component->ParentGraph;
//	bool bSucceedReturn = false;
//	Graph->ExtendNewSplineWithContinuity(bSucceedReturn, Component, false);
//}
//
//bool FRuntimeSplineCommandHelper::CanAddNewSplineAtStart() const
//{
//	return CanAddNewSplineAdj();
//}

void FRuntimeSplineCommandHelper::OnReverseSpline()
{
	auto* Component = ComponentWeakPtr.Get();
	Component->Reverse();
}

bool FRuntimeSplineCommandHelper::CanReverseSpline() const
{
	if (!ComponentWeakPtr.IsValid() || !LastSnappedWorldPosition.IsSet())
	{
		return false;
	}
	if (!IsValid(ComponentWeakPtr->ParentGraph) || ComponentWeakPtr->ParentGraph->IsActorBeingDestroyed())
	{
		return false;
	}
	if (ComponentWeakPtr->PointComponents.Num() < 2)
	{
		return false;
	}
	return true;
}

void FRuntimeSplineCommandHelper::MapActions()
{
	const auto& Commands = FRuntimeSplineCommands::Get();

	CommandList->MapAction(
		Commands.InsertControlPoint,
		FExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::OnInsertControlPoint),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::CanInsertControlPoint));

	CommandList->MapAction(
		Commands.AddNewSplineAtEnd,
		FExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::OnAddNewSplineAdj, true),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::CanAddNewSplineAdj));

	CommandList->MapAction(
		Commands.AddNewSplineAtStart,
		FExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::OnAddNewSplineAdj, false),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::CanAddNewSplineAdj));

	CommandList->MapAction(
		Commands.ReverseSpline,
		FExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::OnReverseSpline),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplineCommandHelper::CanReverseSpline));
}

void FRuntimeSplineCommandHelper::GenerateContextMenuSections(FMenuBuilder& InMenuBuilder) const
{
	InMenuBuilder.BeginSection("SplineEdit", LOCTEXT("Spline", "Spline"));
	{
		InMenuBuilder.AddMenuEntry(FRuntimeSplineCommands::Get().InsertControlPoint);
		// Why assertion failed?
		//InMenuBuilder.AddSubMenu(
		//	LOCTEXT("AddNewSpline", "Add New Spline"),
		//	LOCTEXT("AddNewSplineTooltip", "Add new spline at start or end."),
		//	FNewMenuDelegate::CreateSP(this, &FRuntimeSplineCommandHelper::GenerateContextMenu_AddNewSpline));
		GenerateContextMenu_AddNewSpline(InMenuBuilder);

		InMenuBuilder.AddMenuEntry(FRuntimeSplineCommands::Get().ReverseSpline);
	}
	InMenuBuilder.EndSection();
}

void FRuntimeSplineCommandHelper::GenerateContextMenu_AddNewSpline(FMenuBuilder& InMenuBuilder) const
{
	InMenuBuilder.AddMenuEntry(FRuntimeSplineCommands::Get().AddNewSplineAtEnd);
	InMenuBuilder.AddMenuEntry(FRuntimeSplineCommands::Get().AddNewSplineAtStart);
}

FRuntimeSplineCommands::FRuntimeSplineCommands()
	: TCommands<FRuntimeSplineCommands>
	(
		"RuntimeSplineCommandHelper",	// Context name for fast lookup
		LOCTEXT("RuntimeSplineCommandHelper", "Runtime Spline Command Helper"),	// Localized context name for displaying
		NAME_None,	// Parent
		FRuntimeSplineCommandHelperBase::GetSlateStyle().GetStyleSetName()
	)
{
}

void FRuntimeSplineCommands::RegisterCommands()
{
	UI_COMMAND(InsertControlPoint, "Insert Control Point", "Insert a control point here.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddNewSplineAtEnd, "Add New Spline At End", "Add new spline at end.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(AddNewSplineAtStart, "Add New Spline At Start", "Add new spline at start.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ReverseSpline, "Reverse Spline", "Reverse spline without changing shape.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
