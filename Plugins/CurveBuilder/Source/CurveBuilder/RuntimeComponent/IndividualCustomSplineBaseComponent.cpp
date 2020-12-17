// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "IndividualCustomSplineBaseComponent.h"
#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeSplinePointBaseComponent.h"
#include "SceneProxies/RuntimeCustomSplineSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
//#include "Engine/StaticMesh.h"

UIndividualCustomSplineBaseComponent::UIndividualCustomSplineBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), OriginalMethod(CreationMethod)
{
	//CreationMethod = EComponentCreationMethod::Instance;
	bWantsInitializeComponent = true;
	SplineComponent = CreateDefaultSubobject<URuntimeCustomSplineBaseComponent>(TEXT("RuntimeCustomSplineBaseComponent"));
	//SplineComponent->CreationMethod = EComponentCreationMethod::Instance;
	SplineComponent->SetupAttachment(this);
	TMap<URuntimeSplinePointBaseComponent*, FVector> ConstructSplinePoints
	{
		{
			CreateDefaultSubobject<URuntimeSplinePointBaseComponent>(TEXT("URuntimeSplinePointBaseComponent_0")),
			FVector(0.f, 0.f, 0.f),
		},
		{
			CreateDefaultSubobject<URuntimeSplinePointBaseComponent>(TEXT("URuntimeSplinePointBaseComponent_1")),
			FVector(100.f, 0.f, 0.f),
		},
	};
	for (auto& Pair : ConstructSplinePoints)
	{
		//Pair.Get<0>()->CreationMethod = EComponentCreationMethod::Instance;
		Pair.Get<0>()->SetupAttachment(this);
		SplinePoints.Add(Pair.Get<0>());
	}
	CreateIndividualSpline(
		ConstructSplinePoints,
		ECustomSplineCoordinateType::ComponentLocal,
		ERuntimeSplineType::ClampedBSpline);
}

void UIndividualCustomSplineBaseComponent::OnRegister()
{
	Super::OnRegister();
	//Super::OnAttachmentChanged();
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	bool bIsInstance = CreationMethod == EComponentCreationMethod::Instance;//Debug
	if (IsValid(Owner) && !Owner->IsActorBeingDestroyed())
	{
		CreationMethod = EComponentCreationMethod::Instance;
		//Owner->AddInstanceComponent(this);
		//RegisterComponent();
		//SplineComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		Owner->AddInstanceComponent(SplineComponent);
		//SplineComponent->RegisterComponent();
		for (auto* SplinePointComponent : SplinePoints)
		{
			//SplinePointComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			Owner->AddInstanceComponent(SplinePointComponent);
			//SplinePointComponent->RegisterComponent();
		}
	}
}

void UIndividualCustomSplineBaseComponent::OnUnregister()
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) && !Owner->IsActorBeingDestroyed())
	{
		Owner->ClearInstanceComponents(false);
	}
	//SplineComponent->RegisterComponent();
	for (auto It = SplinePoints.CreateIterator(); It; ++It)
	{
		//SplinePointComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		(*It)->DestroyComponent();
		//SplinePointComponent->RegisterComponent();
	}

	SplineComponent->DestroyComponent();
	//CreationMethod = OriginalMethod;
	Super::OnUnregister();
}

void UIndividualCustomSplineBaseComponent::CreateIndividualSpline(
	const TMap<URuntimeSplinePointBaseComponent*, FVector>& SplinePointsCompToPos,
	ECustomSplineCoordinateType CoordinateType,
	ERuntimeSplineType SplineType)
{
	SplineComponent->bSelected = true;
	SplineComponent->SplineBaseWrapperProxy.Reset();
	SplineComponent->SplineBaseWrapperProxy = MakeShareable(new FSpatialSplineGraph3::FSplineWrapper());
	switch (ARuntimeSplineGraph::GetInternalSplineType(SplineType))
	{
	case ESplineType::ClampedBSpline:
		SplineComponent->SplineBaseWrapperProxy.Get()->Spline = MakeShareable(new TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType());
		break;
	case ESplineType::BezierString:
		SplineComponent->SplineBaseWrapperProxy.Get()->Spline = MakeShareable(new TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType());
		break;
	}
	for (auto& Pair : SplinePointsCompToPos)
	{
		ConstructEndPoint(Pair.Get<0>(), Pair.Get<1>(), true, CoordinateType);
	}
}

void UIndividualCustomSplineBaseComponent::ConstructEndPoint(
	URuntimeSplinePointBaseComponent* PointComponent, 
	const FVector& Position, 
	bool bAtLast, 
	ECustomSplineCoordinateType CoordinateType)
{
	auto* Spline = SplineComponent->GetSplineProxy();
	if (Spline)
	{
		FVector SplineLocalPosition = SplineComponent->ConvertPosition(Position, CoordinateType, ECustomSplineCoordinateType::SplineGraphLocal);
		TWeakPtr<FSpatialControlPoint3> ControlPointWeakPtr;
		if (bAtLast)
		{
			Spline->AddPointAtLast(SplineLocalPosition);
			ControlPointWeakPtr = Spline->GetLastCtrlPointStruct();
		}
		else
		{
			Spline->AddPointAtFirst(SplineLocalPosition);
			ControlPointWeakPtr = Spline->GetLastCtrlPointStruct();
		}
		if (ControlPointWeakPtr.IsValid())
		{
			TSharedRef<FSpatialControlPoint3> ControlPointRef = ControlPointWeakPtr.Pin().ToSharedRef();
			ConstructPointInternal(PointComponent, ControlPointRef, 0);
			//if (Spline->GetType() == ESplineType::BezierString)
			//{
			//	// Need to make connection among three points?
			//	AddPointInternal(ControlPointRef, -1);
			//	AddPointInternal(ControlPointRef, 1);
			//}
		}
	}
}

void UIndividualCustomSplineBaseComponent::ConstructPointInternal(
	URuntimeSplinePointBaseComponent* PointComponent, 
	const TSharedRef<FSpatialControlPoint3>& PointRef, int32 TangentFlag)
{
	auto* Spline = SplineComponent->GetSplineProxy();
	if (Spline)
	{
		//URuntimeSplinePointBaseComponent* NewPoint = PointComponents.Add_GetRef(NewObject<URuntimeSplinePointBaseComponent>());
		//NewPoint->PointIndex = PointComponents.Num() - 1;
		USceneComponent* RealParent = SplineComponent->GetAttachParent();
		if (IsValid(RealParent) && !RealParent->IsBeingDestroyed())
		{
			FTransform SplineLocalToParentComponentLocal = SplineComponent->GetSplineLocalToParentComponentTransform();

			SplineComponent->PointComponents.Add(PointComponent);
			PointComponent->SplinePointProxy = TWeakPtr<FSpatialControlPoint3>(PointRef);
			PointComponent->TangentFlag = TangentFlag;
			PointComponent->ParentSpline = SplineComponent;
			PointComponent->ParentGraph = SplineComponent->ParentGraph; // ParentGraph can be non-exist, which means that this spline is independent.
			//NewPoint->AttachToComponent(RealParent, FAttachmentTransformRules::KeepRelativeTransform);
			//AActor* Owner = GetOwner();
			//if (IsValid(Owner))
			//{
			//	Owner->AddInstanceComponent(NewPoint);
			//	NewPoint->RegisterComponent();
			//}
			//NewPoint->SetRelativeLocation(SplineLocalToParentComponentLocal.TransformPosition(PointRef.Get().Pos)); // Move to Point OnComponentCreated
			PointComponent->SetVisibility(SplineComponent->bSelected);

			//UpdateTransformByCtrlPoint(); // Move to Point OnComponentCreated
		}
	}
}
