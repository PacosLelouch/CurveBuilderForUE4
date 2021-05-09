// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplinePointBaseComponent.h"
#include "RuntimeCustomSplineBaseComponent.h"
#include "SceneProxies/RuntimeSplinePointSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"
#include "Framework/Multibox/MultiboxBuilder.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "RuntimeSplineCommandHelper"

URuntimeSplinePointBaseComponent::URuntimeSplinePointBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	//: Super()
{
	if (ObjectInitializer.GetArchetype())
	{
		bIsDuplicated = true;
	}
	if (bCreateCollisionForSelection)
	{
		CollisionDiameter = DrawPointSize;
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

	InitializeCommandHelper();
}

void URuntimeSplinePointBaseComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	SCOPE_MUTEX_LOCK(RenderMuteX);
	bool bNeedToDestroySpline = false;
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

		if (ParentSpline->PointComponents.Num() < 2 || !ParentSpline->IsRegistered())
		{
			bNeedToDestroySpline = true;
		}
		else
		{
			ParentSpline->UpdateTransformByCtrlPoint();
			ParentSpline->UpdateCollision();
			ParentSpline->OnSplineUpdatedEvent();
		}
	}
	SplinePointProxy.Reset();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	if (bNeedToDestroySpline)
	{
		ParentSpline->DestroyComponent(true);
	}
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

	if (bCreateCollisionForSelection)
	{
		DestroyPhysicsState();
		return;
	}

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

void URuntimeSplinePointBaseComponent::InitializeCommandHelper()
{
	CommandHelper = MakeShareable(new FRuntimeSplinePointCommandHelper(this));
	CommandHelper.Get()->MapActions();
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
				ParentSpline->SetCustomSelected(true);
				//if (IsValid(ParentSpline->SelectedPoint) && !ParentSpline->SelectedPoint->IsBeingDestroyed())
				//{
				//	ParentSpline->SelectedPoint->SetCustomSelected(false);
				//}
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
			ParentGraph->MovePointInternal(ParentSpline, this, Position, CoordinateType);
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

bool URuntimeSplinePointBaseComponent::IsEndPoint(bool& bIsForwardEnd) const
{
	EContactType ContactType = EContactType::End;
	bool bReturnValue = IsEndPointOrNot(ContactType);
	bIsForwardEnd = ContactType == EContactType::End ? true : false;
	return bReturnValue;
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
			SplineLocalPosition = ParentGraph->MovePointInternal(ParentSpline, this, GetRelativeLocation(), ECustomSplineCoordinateType::ComponentLocal);
		}
		else
		{
			ParentSpline->GetSplineProxy()->AdjustCtrlPointPos(*SplinePointProxy.Pin().Get(), GetParentComponentToSplineLocalTransform().TransformPosition(GetRelativeLocation()), TangentFlag, 0);
		}
		ParentSpline->UpdateTransformByCtrlPoint();
		ParentSpline->OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
	}
}

bool URuntimeSplinePointBaseComponent::IsEndPointOrNot(EContactType& OutContactType) const
{
	auto* SplinePointProxyRaw = SplinePointProxy.Pin().Get();
	auto* SplineProxy = ParentSpline->SplineBaseWrapperProxy.Get()->Spline.Get();
	const auto& ParamRange = SplineProxy->GetParamRange();
	FVector PointPos = TVecLib<4>::Projection(SplinePointProxyRaw->Pos),
		StartPos = SplineProxy->GetPosition(ParamRange.Get<0>()),
		EndPos = SplineProxy->GetPosition(ParamRange.Get<1>());

	bool bAtLast = (PointPos - EndPos).IsNearlyZero(1e-3);
	bool bAtFirst = (PointPos - StartPos).IsNearlyZero(1e-3);

	if (!bAtLast && !bAtFirst)
	{
		return false;
	}

	OutContactType = bAtLast ? EContactType::End : EContactType::Start;

	return true;
}

void FRuntimeSplinePointCommandHelper::CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	FRuntimeSplineCommandHelperBase::CapturedMouseMove(InViewport, InMouseX, InMouseY);
}

bool FRuntimeSplinePointCommandHelper::InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	bool bBaseReturnValue = FRuntimeSplineCommandHelperBase::InputKey(Viewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
	return bBaseReturnValue;
}

bool FRuntimeSplinePointCommandHelper::InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	bool bBaseReturnValue = FRuntimeSplineCommandHelperBase::InputAxis(Viewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
	return bBaseReturnValue;
}

bool FRuntimeSplinePointCommandHelper::IsPointAndSplineValid() const
{
	return ComponentWeakPtr.IsValid() && LastSnappedWorldPosition.IsSet() && ComponentWeakPtr->SplinePointProxy.IsValid()
		&& IsValid(ComponentWeakPtr->ParentSpline) && !ComponentWeakPtr->ParentSpline->IsBeingDestroyed()
		&& ComponentWeakPtr->ParentSpline->SplineBaseWrapperProxy.IsValid()
		&& ComponentWeakPtr->ParentSpline->SplineBaseWrapperProxy.Get()->Spline.IsValid();
}

bool FRuntimeSplinePointCommandHelper::IsGraphValid() const
{
	return IsValid(ComponentWeakPtr->ParentGraph) && !ComponentWeakPtr->ParentGraph->IsActorBeingDestroyed();
}

bool FRuntimeSplinePointCommandHelper::CheckPointIsEndPointOrNot(EContactType& OutContactType) const
{
	return ComponentWeakPtr->IsEndPointOrNot(OutContactType);
	//auto* SplinePointProxy = ComponentWeakPtr->SplinePointProxy.Pin().Get();
	//auto* SplineProxy = ComponentWeakPtr->ParentSpline->SplineBaseWrapperProxy.Get()->Spline.Get();
	//const auto& ParamRange = SplineProxy->GetParamRange();
	//FVector PointPos = TVecLib<4>::Projection(SplinePointProxy->Pos),
	//	StartPos = SplineProxy->GetPosition(ParamRange.Get<0>()),
	//	EndPos = SplineProxy->GetPosition(ParamRange.Get<1>());

	//bool bAtLast = (PointPos - EndPos).IsNearlyZero(1e-3);
	//bool bAtFirst = (PointPos - StartPos).IsNearlyZero(1e-3);

	//if (!bAtLast && !bAtFirst)
	//{
	//	return false;
	//}

	//OutContactType = bAtLast ? EContactType::End : EContactType::Start;

	//return true;
}

void FRuntimeSplinePointCommandHelper::OnSplitConnection()
{
	auto* SplinePointProxy = ComponentWeakPtr->SplinePointProxy.Pin().Get();
	auto* SplineProxy = ComponentWeakPtr->ParentSpline->SplineBaseWrapperProxy.Get()->Spline.Get();
	auto& SplineGraphProxy = ComponentWeakPtr->ParentGraph->SplineGraphProxy;
	const auto& ParamRange = SplineProxy->GetParamRange();
	FVector PointPos = TVecLib<4>::Projection(SplinePointProxy->Pos), 
		StartPos = SplineProxy->GetPosition(ParamRange.Get<0>()), 
		EndPos = SplineProxy->GetPosition(ParamRange.Get<1>());

	bool bAtLast = (PointPos - EndPos).IsNearlyZero(1e-3);
	TMap<URuntimeCustomSplineBaseComponent*, bool> AdjacentSplines;
	ComponentWeakPtr->ParentGraph->GetAdjacentSplines(AdjacentSplines, ComponentWeakPtr->ParentSpline, bAtLast);
	for (auto& TargetSplinePair : AdjacentSplines)
	{
		ComponentWeakPtr->ParentGraph->SplitConnection(ComponentWeakPtr->ParentSpline, TargetSplinePair.Get<0>(), bAtLast);
	}
}

bool FRuntimeSplinePointCommandHelper::CanSplitConnection() const
{
	if (!IsPointAndSplineValid())
	{
		return false;
	}
	if (!IsGraphValid())
	{
		return false;
	}

	if (ComponentWeakPtr->TangentFlag != 0)
	{
		return false;
	}

	//auto* SplinePointProxy = ComponentWeakPtr->SplinePointProxy.Pin().Get();
	//auto* SplineProxy = ComponentWeakPtr->ParentSpline->SplineBaseWrapperProxy.Get()->Spline.Get();
	//const auto& ParamRange = SplineProxy->GetParamRange();
	//FVector PointPos = TVecLib<4>::Projection(SplinePointProxy->Pos),
	//	StartPos = SplineProxy->GetPosition(ParamRange.Get<0>()), 
	//	EndPos = SplineProxy->GetPosition(ParamRange.Get<1>());

	//bool bAtLast = (PointPos - EndPos).IsNearlyZero(1e-3);
	//bool bAtFirst = (PointPos - StartPos).IsNearlyZero(1e-3);
	//if (!bAtLast && !bAtFirst)
	//{
	//	return false;
	//}
	EContactType ContactType;
	if (!CheckPointIsEndPointOrNot(ContactType))
	{
		return false;
	}

	if (!ComponentWeakPtr->ParentGraph->CheckSplineHasConnection(ComponentWeakPtr->ParentSpline, ContactType == EContactType::End ? true : false))
	{
		return false;
	}

	return true;
}

void FRuntimeSplinePointCommandHelper::OnConnectAndFillSplines(bool bForward, bool bFillInSource)
{
	EContactType ContactType;
	if (!CheckPointIsEndPointOrNot(ContactType) ||
		ComponentWeakPtr->ParentGraph->SelectedSplines.Num() < 2)
	{
		return;
	}

	TArray<URuntimeCustomSplineBaseComponent*> SelectedOtherSplines;
	SelectedOtherSplines.Reserve(ComponentWeakPtr->ParentGraph->SelectedSplines.Num() - 1);
	bool bFoundParentSpline = false;
	for (URuntimeCustomSplineBaseComponent* SpComp : ComponentWeakPtr->ParentGraph->SelectedSplines)
	{
		if (SpComp == ComponentWeakPtr->ParentSpline)
		{
			bFoundParentSpline = true;
		}
		else
		{
			SelectedOtherSplines.Add(SpComp);
		}
	}

	for (URuntimeCustomSplineBaseComponent* SpComp : SelectedOtherSplines)
	{
		ComponentWeakPtr->ParentGraph->ConnectAndFill(
			SpComp, ComponentWeakPtr->ParentSpline,
			bForward, ContactType == EContactType::End ? true : false, bFillInSource);
	}
}

bool FRuntimeSplinePointCommandHelper::CanConnectAndFillSplines() const
{

	if (!IsPointAndSplineValid())
	{
		return false;
	}
	if (!IsGraphValid())
	{
		return false;
	}

	if (ComponentWeakPtr->TangentFlag != 0)
	{
		return false;
	}

	EContactType ContactType;
	if (!CheckPointIsEndPointOrNot(ContactType) ||
		ComponentWeakPtr->ParentGraph->SelectedSplines.Num() < 2)
	{
		return false;
	}

	return true;
}

void FRuntimeSplinePointCommandHelper::MapActions()
{
	const auto& Commands = FRuntimeSplinePointCommands::Get();

	CommandList->MapAction(
		Commands.SplitConnection,
		FExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::OnSplitConnection),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::CanSplitConnection));

	CommandList->MapAction(
		Commands.ConnectAndFillSplinesEnd,
		FExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::OnConnectAndFillSplines, true, true),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::CanConnectAndFillSplines));

	CommandList->MapAction(
		Commands.ConnectAndFillSplinesStart,
		FExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::OnConnectAndFillSplines, false, true),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::CanConnectAndFillSplines));

	CommandList->MapAction(
		Commands.CreateAndConnectSplinesEnd,
		FExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::OnConnectAndFillSplines, true, false),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::CanConnectAndFillSplines));

	CommandList->MapAction(
		Commands.CreateAndConnectSplinesStart,
		FExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::OnConnectAndFillSplines, false, false),
		FCanExecuteAction::CreateSP(this, &FRuntimeSplinePointCommandHelper::CanConnectAndFillSplines));
}

void FRuntimeSplinePointCommandHelper::GenerateContextMenuSections(FMenuBuilder& InMenuBuilder) const
{
	InMenuBuilder.BeginSection("SplinePointEdit", LOCTEXT("SplinePoint", "Spline Point"));
	{
		InMenuBuilder.AddMenuEntry(FRuntimeSplinePointCommands::Get().SplitConnection);
	}
	InMenuBuilder.EndSection();

	InMenuBuilder.BeginSection("MultipleSplinesEdit", LOCTEXT("MultipleSplines", "Multiple Splines"));
	{
		InMenuBuilder.AddMenuEntry(FRuntimeSplinePointCommands::Get().ConnectAndFillSplinesEnd);
		InMenuBuilder.AddMenuEntry(FRuntimeSplinePointCommands::Get().ConnectAndFillSplinesStart);
		InMenuBuilder.AddMenuEntry(FRuntimeSplinePointCommands::Get().CreateAndConnectSplinesEnd);
		InMenuBuilder.AddMenuEntry(FRuntimeSplinePointCommands::Get().CreateAndConnectSplinesStart);
	}
	InMenuBuilder.EndSection();
}

FRuntimeSplinePointCommands::FRuntimeSplinePointCommands()
	: TCommands<FRuntimeSplinePointCommands>
	(
		"RuntimeSplinePointCommandHelper",	// Context name for fast lookup
		LOCTEXT("RuntimeSplinePointCommandHelper", "Runtime Spline Point Command Helper"),	// Localized context name for displaying
		NAME_None,	// Parent
		FRuntimeSplineCommandHelperBase::GetSlateStyle().GetStyleSetName()
	)
{
}

void FRuntimeSplinePointCommands::RegisterCommands()
{
	UI_COMMAND(SplitConnection, "Split Connection", "Split connection here.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ConnectAndFillSplinesEnd, "Connect And Fill Splines End", "Connect and fill other splines from end to this point.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ConnectAndFillSplinesStart, "Connect And Fill Splines Start", "Connect and fill other splines from start to this point.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CreateAndConnectSplinesEnd, "Create And Connect Splines End", "Create new splines besides other splines and connect from end to this point.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CreateAndConnectSplinesStart, "Create And Connect Splines Start", "Create new splines besides other splines and connect from start to this point.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
