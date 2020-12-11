// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeCustomSplineBaseComponent.h"
#include "PrimitiveSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"

static constexpr bool bDrawLineByCurveLength = false;

template<int32 Dim>
struct TRuntimeSplineDrawInfo<Dim, 3>
{
	TRuntimeSplineDrawInfo(const URuntimeCustomSplineBaseComponent* InComponent)
		: CurveColor(InComponent->CurveColor)
		, CtrlSegColor(InComponent->CtrlSegColor)
		, CtrlPointColor(InComponent->CtrlPointColor)
		, SelectedCtrlPointColor(InComponent->SelectedCtrlPointColor)

		, PointSize(InComponent->PointSize)
		, SegLength(InComponent->SegLength)
		, Thickness(InComponent->Thickness)
		, DepthBias(InComponent->DepthBias)
		, bSelected(InComponent->bSelected)
		, SplineInternalRef(*InComponent->GetSplineProxy())

		//, bDrawDebugCollision(InComponent->bDrawDebugCollision)
		//: SplineComponent(InComponent)
	{}
	FLinearColor CurveColor = FLinearColor::White;
	FLinearColor CtrlSegColor = FLinearColor::White;
	FLinearColor CtrlPointColor = FLinearColor::White;
	FLinearColor SelectedCtrlPointColor = FLinearColor::White;

	float PointSize = 6.f;
	float SegLength = 5.f;
	float Thickness = 0.f;
	float DepthBias = 0.f;
	bool bSelected = false;
	//bool bDrawDebugCollision = false;
	const TSplineBase<Dim, 3>& SplineInternalRef;
	//const URuntimeCustomSplineBaseComponent* SplineComponent;
};

class FRuntimeCustomSplineSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FRuntimeCustomSplineSceneProxy(const URuntimeCustomSplineBaseComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, SplineComponent(InComponent)
	{}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeCustomSplineSceneProxy_GetDynamicMeshElements);

		if (!IsValid(SplineComponent))
		{
			return;
		}

		auto* SplineProxy = SplineComponent->GetSplineProxy();
		if (!SplineProxy)
		{
			return;
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

				const FMatrix& LocalToWorldMat = GetLocalToWorld();
				//const FMatrix& LocalToWorldMat = SplineComponent->GetSplineLocalToWorldMatrix();

				// Taking into account the min and maximum drawing distance
				const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - LocalToWorldMat.GetOrigin()).SizeSquared();
				if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
				{
					continue;
				}

				FSpatial3DrawInfo DrawInfo(SplineComponent);
				
				URuntimeCustomSplineBaseComponent::DrawRuntimeSpline(PDI, View, DrawInfo, LocalToWorldMat, ESceneDepthPriorityGroup::SDPG_Foreground);

				if (SplineComponent->bDrawDebugCollision)
				{
					for (const FKSphylElem& Elem : SplineComponent->BodySetup->AggGeom.SphylElems)
					{
						FTransform LocalTransform(Elem.Rotation, Elem.Center);
						Elem.DrawElemWire(PDI, SplineComponent->BodyInstance.GetUnrealWorldTransform() * LocalTransform, FVector::OneVector, SplineComponent->DebugCollisionColor.ToFColor(true));
					}
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsValid(SplineComponent) && IsShown(View);//bDrawDebug && !IsSelected() && IsShown(View) && View->Family->EngineShowFlags.Splines;
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof *this + GetAllocatedSize(); }
	uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

protected:
	const URuntimeCustomSplineBaseComponent* SplineComponent;
};

void URuntimeCustomSplineBaseComponent::BeginPlay()
{
	Super::BeginPlay(); 
	SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
	SetGenerateOverlapEvents(true);
	CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

FPrimitiveSceneProxy* URuntimeCustomSplineBaseComponent::CreateSceneProxy()
{
	return new FRuntimeCustomSplineSceneProxy(this);
}

FMatrix URuntimeCustomSplineBaseComponent::GetRenderMatrix() const
{
	return GetSplineLocalToWorldMatrix();
}

void URuntimeCustomSplineBaseComponent::OnCreatePhysicsState()
{
	USceneComponent::OnCreatePhysicsState();

	// if we have a scene, we don't want to disable all physics and we have no bodyinstance already
	if (true)//(!BodyInstance.IsValidBodyInstance())
	{
		//UE_LOG(LogPrimitiveComponent, Warning, TEXT("Creating Physics State (%s : %s)"), *GetNameSafe(GetOuter()),  *GetName());

		UBodySetup* UseBodySetup = GetBodySetup();
		if (UseBodySetup)
		{
			// Create new BodyInstance at given location.
			FTransform BodyTransform = GetSplineLocalToWorldTransform();

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
}

void URuntimeCustomSplineBaseComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	UpdateCollision();
}

void URuntimeCustomSplineBaseComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UProperty* PropertyThatChanged = PropertyChangedEvent.Property;
	if (PropertyThatChanged)
	{
		const FName PropertyName = PropertyThatChanged->GetFName();

		if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, SegLength)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, Thickness)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, PointSize)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CurveColor)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CtrlPointColor)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, SelectedCtrlPointColor)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CtrlSegColor)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, bDrawDebugCollision))
		{
			MarkRenderStateDirty();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CollisionSegLength)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(URuntimeCustomSplineBaseComponent, CollisionSegWidth))
		{
			UpdateCollision();
		}
	}
}

void URuntimeCustomSplineBaseComponent::SetSelected(bool bValue)
{
	if (bSelected != bValue)
	{
		bSelected = bValue;
		MarkRenderStateDirty();
	}
}

FSpatialSplineGraph3::FSplineType* URuntimeCustomSplineBaseComponent::GetSplineProxy() const
{
	if (SplineBaseWrapperProxy && SplineBaseWrapperProxy->Spline)
	{
		switch (SplineBaseWrapperProxy->Spline->GetType())
		{
		case ESplineType::ClampedBSpline:
			return GetSplineProxyInternal<TSplineTraitByType<ESplineType::ClampedBSpline>::FSplineType>();
		case ESplineType::BezierString:
			return GetSplineProxyInternal<TSplineTraitByType<ESplineType::BezierString>::FSplineType>();
		}
	}
	return GetSplineProxyInternal<TSplineTraitByType<ESplineType::Unknown>::FSplineType>();
}

FTransform URuntimeCustomSplineBaseComponent::GetSplineLocalToWorldTransform() const
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

FMatrix URuntimeCustomSplineBaseComponent::GetSplineLocalToWorldMatrix() const
{
	return GetSplineLocalToWorldTransform().ToMatrixWithScale();
	//return FMatrix::Identity;
}

FMatrix URuntimeCustomSplineBaseComponent::GetSplineWorldToLocalMatrix() const
{
	return GetSplineLocalToWorldTransform().Inverse().ToMatrixWithScale();
	//return FMatrix::Identity;
}

void URuntimeCustomSplineBaseComponent::UpdateTransformByCtrlPoint()
{
	auto* Spline = GetSplineProxy();
	if (Spline && Spline->GetCtrlPointNum() > 0)
	{
		TTuple<double, double> ParamRange = Spline->GetParamRange();
		SetRelativeLocation(Spline->GetPosition(ParamRange.Get<0>()));
	}
	UpdateCollision();
}

void URuntimeCustomSplineBaseComponent::CreateBodySetup()
{
	if (!IsValid(BodySetup))
	{
		BodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
		BodySetup->BodySetupGuid = FGuid::NewGuid();

		BodySetup->bGenerateMirroredCollision = false;
		BodySetup->bDoubleSidedGeometry = false;
		BodySetup->CollisionTraceFlag = CTF_UseDefault;//bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;
	}
}

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

	FMatrix LocalToWorld = GetSplineLocalToWorldMatrix();
	double T = ParamRange.Get<0>();
	FVector Start = LocalToWorld.TransformPosition(Spline->GetPosition(T));

	// Fill in simple collision sphyl elements
	BodySetup->AggGeom.SphylElems.Empty(SegNum);
	for (int32 i = 0; i < SegNum; ++i)
	{
		T += StepLength;
		FVector End = LocalToWorld.TransformPosition(Spline->GetPosition(T));
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
		SegElem.Rotation = FRotator(-90.f, 0.f, 0.f);
		BodySetup->AggGeom.SphylElems.Add(SegElem);
		Start = End;
	}
	//BodySetup->AggGeom.ConvexElems = CollisionConvexElems;

	// Set trace flag
	BodySetup->CollisionTraceFlag = CTF_UseDefault;// bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

	// New GUID as collision has changed
	BodySetup->BodySetupGuid = FGuid::NewGuid();
	// Also we want cooked data for this
	BodySetup->bHasCookedCollisionData = true;
	BodySetup->InvalidatePhysicsData();
	//BodySetup->CreatePhysicsMeshes();
	RecreatePhysicsState();
}

//template<>
void URuntimeCustomSplineBaseComponent::DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup)
{
	TTuple<double, double> ParamRange = DrawInfo.SplineInternalRef.GetParamRange();
	if (bDrawLineByCurveLength)
	{
		double Length = DrawInfo.SplineInternalRef.GetLength(ParamRange.Get<1>());

		double SegNumDbl = FMath::CeilToDouble(Length / static_cast<double>(DrawInfo.SegLength));//FMath::CeilToDouble(Length * static_cast<double>(DrawInfo.NumSteps));
		int32 SegNum = FMath::RoundToInt(SegNumDbl);
		double StepLength = Length / SegNumDbl;

		double NextS = 0.;
		double T = ParamRange.Get<0>();
		FVector Start = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
		for (int32 i = 0; i < SegNum; ++i)
		{
			NextS += StepLength;
			T = DrawInfo.SplineInternalRef.GetParameterAtLength(NextS);

			FVector End = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
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
		FVector Start = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
		for (int32 i = 0; i < SegNum; ++i)
		{
			T += StepParam;

			FVector End = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
			PDI->DrawLine(Start, End, DrawInfo.CurveColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
			Start = End;
		}
	}

	if (DrawInfo.bSelected)
	{
		switch (DrawInfo.SplineInternalRef.GetType())
		{
		case ESplineType::ClampedBSpline:
		{
			const auto& BSpline = static_cast<const TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType&>(DrawInfo.SplineInternalRef);
			TArray<FVector4> CtrlPoints;
			BSpline.GetCtrlPoints(CtrlPoints);

			if (CtrlPoints.Num() > 1)
			{
				FVector Start = LocalToWorld.TransformPosition(CtrlPoints[0]);
				PDI->DrawPoint(Start, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
				for (int32 i = 1; i < CtrlPoints.Num(); ++i)
				{
					FVector End = LocalToWorld.TransformPosition(CtrlPoints[i]);
					PDI->DrawLine(Start, End, DrawInfo.CtrlSegColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
					Start = End;
					PDI->DrawPoint(Start, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
				}
			}
		}
		break;
		case ESplineType::BezierString:
		{
			const auto& BezierString = static_cast<const TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType&>(DrawInfo.SplineInternalRef);
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

					PDI->DrawPoint(Start, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
					PDI->DrawPoint(Prev, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);
					PDI->DrawPoint(Next, DrawInfo.CtrlPointColor, DrawInfo.PointSize, DepthPriorityGroup);

					PDI->DrawLine(Start, Prev, DrawInfo.CtrlSegColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
					PDI->DrawLine(Start, Next, DrawInfo.CtrlSegColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
				}
			}
		}
		break;
		}
	}
}
