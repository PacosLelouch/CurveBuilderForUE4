// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "RuntimeSplinePrimitiveComponent.h"
#include "SceneProxies/RuntimeSplinePrimitiveSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"
#include "Styling/SlateStyle.h"
#include "Styling/CoreStyle.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandInfo.h"

#define LOCTEXT_NAMESPACE "RuntimeSplineCommandHelper"

DEFINE_LOG_CATEGORY_STATIC(LogRuntimeSplinePrimitiveComponent, Warning, All)

TSharedRef<ISlateStyle> FRuntimeSplineCommands::SlateStyle = FCoreStyle::Create();

ECollisionTraceFlag URuntimeSplinePrimitiveComponent::SpCompCollisionTraceFlag = ECollisionTraceFlag::CTF_UseSimpleAsComplex;

URuntimeSplinePrimitiveComponent::URuntimeSplinePrimitiveComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseAttachParentBound = false;
	bAlwaysCreatePhysicsState = true;
	CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	DepthPriorityGroup = ESceneDepthPriorityGroup::SDPG_Foreground;
}

void URuntimeSplinePrimitiveComponent::BeginPlay()
{
	Super::BeginPlay();
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetGenerateOverlapEvents(true);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

FPrimitiveSceneProxy* URuntimeSplinePrimitiveComponent::CreateSceneProxy()
{
	return new FRuntimeSplinePrimitiveSceneProxy(this);
}

FMatrix URuntimeSplinePrimitiveComponent::GetRenderMatrix() const
{
	return GetSplineLocalToWorldTransform().ToMatrixWithScale();
}

void URuntimeSplinePrimitiveComponent::OnCreatePhysicsState()
{
#if true
	Super::OnCreatePhysicsState();
#else
	USceneComponent::OnCreatePhysicsState();

	// if we have a scene, we don't want to disable all physics and we have no bodyinstance already
	//if (true)
	if (!BodyInstance.IsValidBodyInstance())
	{
		//UE_LOG(LogPrimitiveComponent, Warning, TEXT("Creating Physics State (%s : %s)"), *GetNameSafe(GetOuter()),  *GetName());

		UBodySetup* UseBodySetup = GetBodySetup();
		if (UseBodySetup)
		{
			// Create new BodyInstance at given location.
			FTransform BodyTransform = GetComponentTransform();//GetSplineLocalToWorldTransform();

			// Here we make sure we don't have zero scale. This still results in a body being made and placed in
			// world (very small) but is consistent with a body scaled to zero.
			const FVector BodyScale = BodyTransform.GetScale3D();
			if (BodyScale.IsNearlyZero())
			{
				BodyTransform.SetScale3D(FVector(KINDA_SMALL_NUMBER));
			}
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if ((BodyInstance.GetCollisionEnabled() != ECollisionEnabled::NoCollision) && (FMath::IsNearlyZero(BodyScale.X) || FMath::IsNearlyZero(BodyScale.Y) || FMath::IsNearlyZero(BodyScale.Z)))
			{
				UE_LOG(LogPhysics, Warning, TEXT("Scale for %s has a component set to zero, which will result in a bad body instance. Scale:%s"), *GetPathNameSafe(this), *BodyScale.ToString());
			}
#endif

			// Create the body.
			BodyInstance.InitBody(UseBodySetup, BodyTransform, this, GetWorld()->GetPhysicsScene());
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			SendRenderDebugPhysics();
#endif // !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

#if WITH_EDITOR
			// Make sure we have a valid body instance here. As we do not keep BIs with no collision shapes at all,
			// we don't want to create cloth collision in these cases
			if (BodyInstance.IsValidBodyInstance())
			{
				const float RealMass = BodyInstance.GetBodyMass();
				const float CalcedMass = BodySetup->CalculateMass(this);
				float MassDifference = RealMass - CalcedMass;
				if (RealMass > 1.0f && FMath::Abs(MassDifference) > 0.1f)
				{
					UE_LOG(LogPhysics, Log, TEXT("Calculated mass differs from real mass for %s:%s. Mass: %f  CalculatedMass: %f"),
						GetOwner() != NULL ? *GetOwner()->GetName() : TEXT("NoActor"),
						*GetName(), RealMass, CalcedMass);
				}
			}
#endif // WITH_EDITOR
		}
	}
#endif
}

void URuntimeSplinePrimitiveComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	UpdateBounds();
	UpdateCollision();
	MarkRenderStateDirty();
	//MarkRenderTransformDirty();  // Need to send new bounds to render thread
}

FBoxSphereBounds URuntimeSplinePrimitiveComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	UE_LOG(LogRuntimeSplinePrimitiveComponent, Warning, TEXT("CalcBounds not override."));
	return Super::CalcBounds(LocalToWorld);
}
void URuntimeSplinePrimitiveComponent::SetDrawDebugCollision(bool bValue)
{
	if (bDrawDebugCollision != bValue)
	{
		bDrawDebugCollision = bValue;
		MarkRenderStateDirty();
	}
}

void URuntimeSplinePrimitiveComponent::SetDrawInGame(bool bValue)
{
	if (bDrawInGame != bValue)
	{
		bDrawInGame = bValue;
		MarkRenderStateDirty();
	}
}

#if WITH_EDITOR
void URuntimeSplinePrimitiveComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//UE_LOG(LogRuntimeSplinePrimitiveComponent, Warning, TEXT("PostEditChangeProperty not override."));
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeSplinePrimitiveComponent, bDrawInGame)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeSplinePrimitiveComponent, bDrawDebugCollision))
	{
		MarkRenderStateDirty();
	}
}
#endif

FTransform URuntimeSplinePrimitiveComponent::GetSplineLocalToWorldTransform() const
{
	if (ParentGraph)
	{
		return ParentGraph->GetActorTransform();
	}

	USceneComponent* RealParent = GetAttachParent();
	if (IsValid(RealParent))
	{
		return RealParent->GetComponentTransform();
	}

	return GetComponentTransform();
}

FTransform URuntimeSplinePrimitiveComponent::GetWorldToSplineLocalTransform() const
{
	return GetSplineLocalToWorldTransform().Inverse();
}

FTransform URuntimeSplinePrimitiveComponent::GetSplineLocalToComponentLocalTransform() const
{
	return GetSplineLocalToWorldTransform() * GetComponentTransform().Inverse();
}

FTransform URuntimeSplinePrimitiveComponent::GetComponentLocalToSplineLocalTransform() const
{
	return GetComponentTransform() * GetSplineLocalToWorldTransform().Inverse();
}

FTransform URuntimeSplinePrimitiveComponent::GetWorldToParentComponentTransform() const
{
	return GetParentComponentToWorldTransform().Inverse();
}

FTransform URuntimeSplinePrimitiveComponent::GetParentComponentToWorldTransform() const
{
	USceneComponent* RealParent = GetAttachParent();
	if (IsValid(RealParent))
	{
		return RealParent->GetComponentTransform();
	}

	return GetComponentTransform();
}

FTransform URuntimeSplinePrimitiveComponent::GetSplineLocalToParentComponentTransform() const
{
	USceneComponent* RealParent = GetAttachParent();
	if (IsValid(RealParent))
	{
		return GetSplineLocalToWorldTransform() * RealParent->GetComponentTransform().Inverse();
	}
	return GetSplineLocalToWorldTransform();
}

FTransform URuntimeSplinePrimitiveComponent::GetParentComponentToSplineLocalTransform() const
{
	USceneComponent* RealParent = GetAttachParent();
	if (IsValid(RealParent))
	{
		return RealParent->GetComponentTransform() * GetWorldToSplineLocalTransform();
	}
	return GetWorldToSplineLocalTransform();
}

FVector URuntimeSplinePrimitiveComponent::ConvertPosition(const FVector& SourcePosition, ECustomSplineCoordinateType From, ECustomSplineCoordinateType To) const
{
	switch (From)
	{
	case ECustomSplineCoordinateType::ComponentLocal:
		switch (To)
		{
		case ECustomSplineCoordinateType::SplineGraphLocal:
			return GetParentComponentToSplineLocalTransform().TransformPosition(SourcePosition);
		case ECustomSplineCoordinateType::World:
			return GetParentComponentToWorldTransform().TransformPosition(SourcePosition);
		}
		break;
	case ECustomSplineCoordinateType::SplineGraphLocal:
		switch (To)
		{
		case ECustomSplineCoordinateType::ComponentLocal:
			return GetSplineLocalToParentComponentTransform().TransformPosition(SourcePosition);
		case ECustomSplineCoordinateType::World:
			return GetSplineLocalToWorldTransform().TransformPosition(SourcePosition);
		}
		break;
	case ECustomSplineCoordinateType::World:
		switch (To)
		{
		case ECustomSplineCoordinateType::SplineGraphLocal:
			return GetWorldToSplineLocalTransform().TransformPosition(SourcePosition);
		case ECustomSplineCoordinateType::ComponentLocal:
			return GetWorldToParentComponentTransform().TransformPosition(SourcePosition);
		}
		break;
	}
	return SourcePosition;
}

void URuntimeSplinePrimitiveComponent::CreateBodySetup()
{
	if (!IsValid(BodySetup))
	{
		BodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
		BodySetup->BodySetupGuid = FGuid::NewGuid();

		BodySetup->BuildScale3D = FVector::OneVector;
		BodySetup->bGenerateMirroredCollision = false;
		BodySetup->bDoubleSidedGeometry = false;
		BodySetup->CollisionTraceFlag = SpCompCollisionTraceFlag;
	}
}

void URuntimeSplinePrimitiveComponent::UpdateCollision()
{
	UE_LOG(LogRuntimeSplinePrimitiveComponent, Warning, TEXT("UpdateCollision not override."));
}

FRuntimeSplineCommands::FRuntimeSplineCommands()
	: TCommands<FRuntimeSplineCommands>
	(
		"RuntimeSplineCommandHelper",	// Context name for fast lookup
		LOCTEXT("RuntimeSplineCommandHelper", "Runtime Spline Command Helper"),	// Localized context name for displaying
		NAME_None,	// Parent
		SlateStyle.Get().GetStyleSetName()
	)
{
}

void FRuntimeSplineCommands::RegisterCommands()
{
	//UI_COMMAND(DeleteKey, "Delete Spline Point", "Delete the currently selected spline point.", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
	//UI_COMMAND(DuplicateKey, "Duplicate Spline Point", "Duplicate the currently selected spline point.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(AddKey, "Add Spline Point Here", "Add a new spline point at the cursor location.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(SelectAll, "Select All Spline Points", "Select all spline points.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(ResetToUnclampedTangent, "Unclamped Tangent", "Reset the tangent for this spline point to its default unclamped value.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(ResetToClampedTangent, "Clamped Tangent", "Reset the tangent for this spline point to its default clamped value.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(SetKeyToCurve, "Curve", "Set spline point to Curve type", EUserInterfaceActionType::RadioButton, FInputChord());
	//UI_COMMAND(SetKeyToLinear, "Linear", "Set spline point to Linear type", EUserInterfaceActionType::RadioButton, FInputChord());
	//UI_COMMAND(SetKeyToConstant, "Constant", "Set spline point to Constant type", EUserInterfaceActionType::RadioButton, FInputChord());
	//UI_COMMAND(FocusViewportToSelection, "Focus Selected", "Moves the camera in front of the selection", EUserInterfaceActionType::Button, FInputChord(EKeys::F));
	//UI_COMMAND(SnapToNearestSplinePoint, "Snap to Nearest Spline Point", "Snap to nearest spline point.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(AlignToNearestSplinePoint, "Align to Nearest Spline Point", "Align to nearest spline point.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(AlignPerpendicularToNearestSplinePoint, "Align Perpendicular to Nearest Spline Point", "Align perpendicular to nearest spline point.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(SnapAllToSelectedX, "Snap All To Selected X", "Snap all spline points to selected spline point X.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(SnapAllToSelectedY, "Snap All To Selected Y", "Snap all spline points to selected spline point Y.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(SnapAllToSelectedZ, "Snap All To Selected Z", "Snap all spline points to selected spline point Z.", EUserInterfaceActionType::Button, FInputChord());
	//UI_COMMAND(SetLockedAxisNone, "None", "New spline point axis is not fixed.", EUserInterfaceActionType::RadioButton, FInputChord());
	//UI_COMMAND(SetLockedAxisX, "X", "Fix X axis when adding new spline points.", EUserInterfaceActionType::RadioButton, FInputChord());
	//UI_COMMAND(SetLockedAxisY, "Y", "Fix Y axis when adding new spline points.", EUserInterfaceActionType::RadioButton, FInputChord());
	//UI_COMMAND(SetLockedAxisZ, "Z", "Fix Z axis when adding new spline points.", EUserInterfaceActionType::RadioButton, FInputChord());
	//UI_COMMAND(VisualizeRollAndScale, "Visualize Roll and Scale", "Whether the visualization should show roll and scale on this spline.", EUserInterfaceActionType::ToggleButton, FInputChord());
	//UI_COMMAND(DiscontinuousSpline, "Allow Discontinuous Splines", "Whether the visualization allows Arrive and Leave tangents to be set separately.", EUserInterfaceActionType::ToggleButton, FInputChord());
	//UI_COMMAND(ResetToDefault, "Reset to Default", "Reset this spline to its archetype default.", EUserInterfaceActionType::Button, FInputChord());
}

void FRuntimeSplineCommandHelperBase::CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	UE_LOG(LogRuntimeSplinePrimitiveComponent, Log, TEXT("FRuntimeSplineCommandHelperBase::CapturedMouseMove: <%d, %d>"), InMouseX, InMouseY);
}

bool FRuntimeSplineCommandHelperBase::InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	UE_LOG(LogRuntimeSplinePrimitiveComponent, Log, TEXT("FRuntimeSplineCommandHelperBase::InputKey: CtrlId: %d, Key: %s, Event: %d, AmountDepressed: %f, Gamepad: %d"),
		ControllerId, *Key.ToString(), Event, AmountDepressed, bGamepad);
	return false;
}

bool FRuntimeSplineCommandHelperBase::InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	UE_LOG(LogRuntimeSplinePrimitiveComponent, Log, TEXT("FRuntimeSplineCommandHelperBase::InputAxis: CtrlId: %d, Key: %s, Delta: %f, DeltaTime: %f, NumSamples: %d, Gamepad: %d"),
		ControllerId, *Key.ToString(), Delta, DeltaTime, NumSamples, bGamepad);
	return false;
}

#undef LOCTEXT_NAMESPACE
