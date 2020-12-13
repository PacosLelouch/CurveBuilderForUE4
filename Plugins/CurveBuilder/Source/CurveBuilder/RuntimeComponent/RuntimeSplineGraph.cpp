// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.h"

void USplineGraphRootComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	ARuntimeSplineGraph* Graph = Cast<ARuntimeSplineGraph>(GetOwner());
	if (IsValid(Graph))
	{
		for (auto& SpPair : Graph->SplineComponentMap)
		{
			URuntimeCustomSplineBaseComponent* Comp = SpPair.Get<1>();
			Comp->UpdateCollision();
			Comp->MarkRenderStateDirty();
		}
	}
}

ARuntimeSplineGraph::ARuntimeSplineGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	USplineGraphRootComponent* NewRootComponent = CreateDefaultSubobject<USplineGraphRootComponent>(TEXT("RootComponent"));
	RootComponent = NewRootComponent;
}

void ARuntimeSplineGraph::Destroyed()
{
	SplineGraphProxy.Empty();
	SplineComponentMap.Empty();
	Super::Destroyed();
}

void ARuntimeSplineGraph::VirtualAttachSplineComponent(URuntimeCustomSplineBaseComponent* SplineComponent)
{
	if (IsValid(SplineComponent))
	{
		SplineComponent->ParentGraph = this;
		SplineComponentMap.Add(SplineComponent->SplineBaseWrapperProxy, SplineComponent);
		SplineComponent->UpdateTransformByCtrlPoint();
	}
}

void ARuntimeSplineGraph::GetOwningSplines(TArray<URuntimeCustomSplineBaseComponent*>& Splines)
{
	Splines.Empty(SplineComponentMap.Num());
	for (auto& SpPair : SplineComponentMap)
	{
		Splines.Add(SpPair.Get<1>());
	}
}

URuntimeCustomSplineBaseComponent* ARuntimeSplineGraph::GetSplineComponentBySplineWeakPtr(TWeakPtr<FSpatialSplineGraph3::FSplineType> SplineWeakPtr)
{
	TWeakPtr<FSpatialSplineGraph3::FSplineWrapper> WrapperWeakPtr = SplineGraphProxy.GetSplineWrapper(SplineWeakPtr);
	if (!WrapperWeakPtr.IsValid())
	{
		return nullptr;
	}
	URuntimeCustomSplineBaseComponent** SpCompPtr = SplineComponentMap.Find(WrapperWeakPtr.Pin());
	if (!SpCompPtr)
	{
		return nullptr;
	}
	return *SpCompPtr;
}

#if WITH_EDITOR
void USplineGraphRootComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const static FName LocationName("RelativeLocation");
	const static FName RotationName("RelativeRotation");
	const static FName ScaleName("RelativeScale3D");

	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	
	const bool bLocationChanged = (PropertyName == LocationName || MemberPropertyName == LocationName);
	
	if (bLocationChanged || (PropertyName == RotationName || MemberPropertyName == RotationName) || (PropertyName == ScaleName || MemberPropertyName == ScaleName))
	{
		OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
	}
}
#endif
