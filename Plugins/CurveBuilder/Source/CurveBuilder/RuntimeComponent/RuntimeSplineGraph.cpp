// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeSplinePointBaseComponent.h"

void USplineGraphRootComponent::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	ARuntimeSplineGraph* Graph = Cast<ARuntimeSplineGraph>(GetOwner());
	if (IsValid(Graph))
	{
		for (auto& SpPair : Graph->SplineComponentMap)
		{
			URuntimeCustomSplineBaseComponent* Comp = SpPair.Get<1>();
			if (IsValid(Comp))
			{
				//Comp->OnUpdateTransform(UpdateTransformFlags, Teleport);
				for (URuntimeSplinePointBaseComponent* PC : Comp->PointComponents)
				{
					PC->UpdateComponentLocationBySpline();
					//PC->MoveSplinePointInternal();
					//PC->OnUpdateTransform(UpdateTransformFlags, Teleport);
				}
				Comp->UpdateTransformByCtrlPoint();
			}
		}
	}
}

ARuntimeSplineGraph::ARuntimeSplineGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActorWithSpline = AActor::StaticClass();
	CustomSplineClass = URuntimeCustomSplineBaseComponent::StaticClass();
	CustomSplinePointClass = URuntimeSplinePointBaseComponent::StaticClass();
	SplineGraphRootComponent = CreateDefaultSubobject<USplineGraphRootComponent>(TEXT("RootComponent"));
	RootComponent = SplineGraphRootComponent;
}

void ARuntimeSplineGraph::Destroyed()
{
	ClearAllSplines();
	Super::Destroyed();
}

void ARuntimeSplineGraph::VirtualAttachSplineComponent(URuntimeCustomSplineBaseComponent* SplineComponent)
{
	if (IsValid(SplineComponent) && !SplineComponent->IsBeingDestroyed())
	{
		SplineComponent->ParentGraph = this;
		SplineComponent->CustomSplinePointClass = CustomSplinePointClass;
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

bool ARuntimeSplineGraph::CheckSplineHasConnection(URuntimeCustomSplineBaseComponent* SplineComponent, bool bAtLast)
{
	if (IsValid(SplineComponent) && !SplineComponent->IsBeingDestroyed())
	{
		TWeakPtr<FSpatialSplineBase3> SplineWeakPtr = SplineComponent->GetSplineProxyWeakPtr();
		return SplineGraphProxy.HasConnection(SplineWeakPtr, bAtLast ? EContactType::End : EContactType::Start);
	}
	return false;
}

void ARuntimeSplineGraph::GetAdjacentSplines(TMap<URuntimeCustomSplineBaseComponent*, bool>& OutAdjacentSplinesAndForward, URuntimeCustomSplineBaseComponent* SourceSpline, bool bForward)
{
	if (IsValid(SourceSpline) && !SourceSpline->IsBeingDestroyed())
	{
		TArray<FSpatialSplineGraph3::FGraphNode> ConnectedSplineNodes;
		//TArray<TWeakPtr<FSpatialSplineGraph3::FSplineWrapper> > ConnectedSplineWrappers;
		SplineGraphProxy.HasConnection(SourceSpline->GetSplineProxyWeakPtr(), bForward ? EContactType::End : EContactType::Start, &ConnectedSplineNodes);
		OutAdjacentSplinesAndForward.Empty(ConnectedSplineNodes.Num());
		for (const auto& Node : ConnectedSplineNodes)
		{
			URuntimeCustomSplineBaseComponent** CompPtr = SplineComponentMap.Find(Node.SplineWrapper.Pin());
			if (CompPtr)
			{
				OutAdjacentSplinesAndForward.Add(*CompPtr, Node.ContactType == EContactType::End ? true : false);
			}
		}
	}
}

void ARuntimeSplineGraph::GetClusterSplinesWithoutSource(TMap<URuntimeCustomSplineBaseComponent*, int32>& OutClusterSplinesWithDistance, URuntimeCustomSplineBaseComponent* SourceSpline, bool bForward)
{
	if (IsValid(SourceSpline) && !SourceSpline->IsBeingDestroyed())
	{
		TSharedPtr<FSpatialSplineBase3> SplineSharedPtr = SourceSpline->GetSplineProxyWeakPtr().Pin();
		TSet<TTuple<FSpatialSplineGraph3::FGraphNode, int> > ConnectedSplineWrappers;
		SplineGraphProxy.GetClusterWithoutSelf(ConnectedSplineWrappers, SplineSharedPtr, bForward ? EContactType::End : EContactType::Start);
		OutClusterSplinesWithDistance.Empty(ConnectedSplineWrappers.Num());
		for (const auto& Tuple : ConnectedSplineWrappers)
		{
			URuntimeCustomSplineBaseComponent** CompPtr = SplineComponentMap.Find(Tuple.Get<0>().SplineWrapper.Pin());
			if (CompPtr)
			{
				OutClusterSplinesWithDistance.Add(*CompPtr, Tuple.Get<1>());
			}
		}
	}
}

void ARuntimeSplineGraph::ClearAllSplines()
{
	for (auto& SpPair : SplineComponentMap)
	{
		SpPair.Get<1>()->DestroyComponent();
	}
	SplineGraphProxy.Empty();
	SplineComponentMap.Empty();
}

void ARuntimeSplineGraph::SplitConnection(URuntimeCustomSplineBaseComponent* Source, URuntimeCustomSplineBaseComponent* Target, bool bForward)
{
	if (!IsValid(Source) || Source->IsBeingDestroyed() || !IsValid(Target) || Target->IsBeingDestroyed())
	{
		return;
	}
	SplineGraphProxy.SplitConnection(
		Target->SplineBaseWrapperProxy.Get()->Spline,
		Source->SplineBaseWrapperProxy.Get()->Spline,
		bForward ? EContactType::End : EContactType::Start);
}

URuntimeCustomSplineBaseComponent* ARuntimeSplineGraph::ConnectAndFill(
	URuntimeCustomSplineBaseComponent* Source, URuntimeCustomSplineBaseComponent* Target, 
	bool bSourceForward, bool bTargetForward, 
	bool bFillInSource)
{
	if (!IsValid(Source) || Source->IsBeingDestroyed() || !IsValid(Target) || Target->IsBeingDestroyed())
	{
		return nullptr;
	}
	TWeakPtr<FSpatialSplineBase3> ReturnSplineWeakPtr = SplineGraphProxy.ConnectAndFill(
		Source->SplineBaseWrapperProxy.Get()->Spline, Target->SplineBaseWrapperProxy.Get()->Spline,
		bSourceForward ? EContactType::End : EContactType::Start,
		bTargetForward ? EContactType::End : EContactType::Start,
		bFillInSource);

	if (!ReturnSplineWeakPtr.IsValid())
	{
		return nullptr;
	}

	if (!bFillInSource)
	{
		auto* NewSplineComponent = CreateSplineActorInternal(ReturnSplineWeakPtr);
		return NewSplineComponent;
	}
	else // Fill in source. Need to bind new points.
	{
		TArray<TWeakPtr<FSpatialControlPoint3> > NewCtrlPointStructs;
		auto* SourceSpline = ReturnSplineWeakPtr.Pin().Get();
		switch (SourceSpline->GetType())
		{
		case ESplineType::ClampedBSpline:
		{
			auto* SBSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType*>(SourceSpline);
			TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType::FPointNode* FirstNode = nullptr;
			TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType::FPointNode* SecondNode = nullptr;
			if (bSourceForward)
			{
				SecondNode = SBSpline->LastNode();
				FirstNode = SecondNode->GetPrevNode();
			}
			else
			{
				SecondNode = SBSpline->FirstNode();
				FirstNode = SecondNode->GetNextNode();
			}
			NewCtrlPointStructs.Add(FirstNode->GetValue());
			NewCtrlPointStructs.Add(SecondNode->GetValue());
		}
			break;
		case ESplineType::BezierString:
		{
			auto* SBeziers = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType*>(SourceSpline);
			TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType::FPointNode* FirstNode = nullptr;
			if (bSourceForward)
			{
				FirstNode = SBeziers->LastNode();
			}
			else
			{
				FirstNode = SBeziers->FirstNode();
			}
			NewCtrlPointStructs.Add(FirstNode->GetValue());
		}
			break;
		}

		AddUnbindingPointsInternal(NewCtrlPointStructs, Source);
	}
	return Source;
}

void ARuntimeSplineGraph::RemoveSplineFromGraph(URuntimeCustomSplineBaseComponent* SplineToDelete)
{
	if (!IsValid(SplineToDelete) || !SplineToDelete->SplineBaseWrapperProxy.IsValid())
	{
		return;
	}

	URuntimeCustomSplineBaseComponent** CompPtr = SplineComponentMap.Find(SplineToDelete->SplineBaseWrapperProxy);
	if (CompPtr && *CompPtr == SplineToDelete)
	{
		SplineGraphProxy.DeleteSpline(SplineToDelete->SplineBaseWrapperProxy.Get()->Spline);
		SplineComponentMap.Remove(SplineToDelete->SplineBaseWrapperProxy);
	}
}

URuntimeCustomSplineBaseComponent* ARuntimeSplineGraph::CreateNewActorWithEmptySpline(ERuntimeSplineType SplineTypeToCreate)
{
	TWeakPtr<FSpatialSplineBase3> SplineWeakPtr = SplineGraphProxy.AddDefaulted(GetInternalSplineType(SplineTypeToCreate));

	URuntimeCustomSplineBaseComponent* NewSpline = CreateSplineActorInternal(SplineWeakPtr);
	//UWorld* World = GetWorld();
	//if (!IsValid(World))
	//{
	//	return nullptr;
	//}

	//AActor* NewActor = World->SpawnActor<AActor>(ActorWithSpline, GetActorTransform());
	//if (!IsValid(NewActor))
	//{
	//	return nullptr;
	//}
	////if (ActorWithSpline.Get() == AActor::StaticClass()))
	//if (!IsValid(NewActor->GetRootComponent()))
	//{
	//	USceneComponent* NewRootComponent = NewObject<USceneComponent>(NewActor);
	//	NewActor->SetRootComponent(NewRootComponent);
	//	NewActor->AddInstanceComponent(NewRootComponent);
	//	NewRootComponent->RegisterComponent();
	//}

	//URuntimeCustomSplineBaseComponent* NewSpline = NewObject<URuntimeCustomSplineBaseComponent>(NewActor, CustomSplineClass);
	//NewSpline->AttachToComponent(NewActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	//NewActor->AddInstanceComponent(NewSpline);
	//NewSpline->RegisterComponent();
	//NewSpline->SplineBaseWrapperProxy = SplineGraphProxy.GetSplineWrapper(SplineWeakPtr).Pin();

	//VirtualAttachSplineComponent(NewSpline);

	return NewSpline;
}

URuntimeSplinePointBaseComponent* ARuntimeSplineGraph::AddEndPoint(
	const FVector& Position,
	URuntimeCustomSplineBaseComponent* ToSpline,
	bool bAtLast, 
	ECustomSplineCoordinateType CoordinateType,
	ERuntimeSplineType SplineTypeToCreate)
{
	if (!IsValid(ToSpline))
	{
		ToSpline = CreateNewActorWithEmptySpline(SplineTypeToCreate);
	}

	if (!SplineComponentMap.Contains(ToSpline->SplineBaseWrapperProxy))
	{
		SplineComponentMap.Add(ToSpline->SplineBaseWrapperProxy, ToSpline);
	}
	
	return ToSpline->AddEndPoint(Position, bAtLast, CoordinateType);
}

URuntimeSplinePointBaseComponent* ARuntimeSplineGraph::InsertPoint(
	const FVector& Position,
	bool& bSucceedReturn,
	URuntimeCustomSplineBaseComponent* ToSpline, 
	ECustomSplineCoordinateType CoordinateType)
{
	if (IsValid(ToSpline) && !ToSpline->IsBeingDestroyed())
	{
		URuntimeSplinePointBaseComponent* ReturnComponent = ToSpline->InsertPoint(Position, bSucceedReturn, CoordinateType);
		if (!ReturnComponent)
		{
			ReturnComponent = AddEndPoint(Position, ToSpline, true, CoordinateType, ToSpline->GetSplineType());
		}

		bSucceedReturn = (ReturnComponent != nullptr);
		return ReturnComponent;
	}

	bSucceedReturn = false;
	return nullptr;
}

URuntimeCustomSplineBaseComponent* ARuntimeSplineGraph::ExtendNewSplineWithContinuity(bool& bSucceedReturn, URuntimeCustomSplineBaseComponent* SourceSpline, bool bAtLast, ECustomSplineCoordinateType CoordinateType)
{
	if (IsValid(SourceSpline) && !SourceSpline->IsBeingDestroyed())
	{
		TWeakPtr<FSpatialSplineBase3> SplineWeakPtr = SourceSpline->GetSplineProxyWeakPtr();
		TArray<TWeakPtr<FSpatialControlPoint3> > ControlPointStructsAddedInSourceSpline;
		TWeakPtr<FSpatialSplineBase3> NewSplineWeakPtr = SplineGraphProxy.CreateSplineBesidesExisted(SplineWeakPtr, bAtLast ? EContactType::End : EContactType::Start, 1, &ControlPointStructsAddedInSourceSpline);
		AddUnbindingPointsInternal(ControlPointStructsAddedInSourceSpline, SourceSpline, nullptr);

		if (!NewSplineWeakPtr.IsValid())
		{
			bSucceedReturn = false;
			return nullptr;
		}

		URuntimeCustomSplineBaseComponent* NewSplineComp = CreateSplineActorInternal(NewSplineWeakPtr, nullptr);

		bSucceedReturn = (NewSplineComp != nullptr);

		if (bSucceedReturn)
		{
			SourceSpline->SetCustomSelected(false);
		}
		return NewSplineComp;
	}

	bSucceedReturn = false;
	return nullptr;
}

URuntimeSplinePointBaseComponent* ARuntimeSplineGraph::ExtendNewSplineAndNewPoint(
	const FVector& Position,
	bool& bSucceedReturn,
	URuntimeCustomSplineBaseComponent* SourceSpline,
	bool bAtLast,
	ECustomSplineCoordinateType CoordinateType)
{
	if (IsValid(SourceSpline) && !SourceSpline->IsBeingDestroyed())
	{
		TWeakPtr<FSpatialSplineBase3> SplineWeakPtr = SourceSpline->GetSplineProxyWeakPtr();
		TArray<TWeakPtr<FSpatialControlPoint3> > ControlPointStructsAddedInSourceSpline;
		TWeakPtr<FSpatialSplineBase3> NewSplineWeakPtr = SplineGraphProxy.CreateSplineBesidesExisted(SplineWeakPtr, bAtLast ? EContactType::End : EContactType::Start, 1, &ControlPointStructsAddedInSourceSpline);
		AddUnbindingPointsInternal(ControlPointStructsAddedInSourceSpline, SourceSpline, nullptr);

		if (!NewSplineWeakPtr.IsValid())
		{
			bSucceedReturn = false;
			return nullptr;
		}

		URuntimeSplinePointBaseComponent* LatestNewPoint = nullptr;
		URuntimeCustomSplineBaseComponent* NewSplineComp = CreateSplineActorInternal(NewSplineWeakPtr, &LatestNewPoint);
		FSpatialSplineBase3& SplineRef = *NewSplineWeakPtr.Pin().Get();

		switch (SplineRef.GetType())
		{
		case ESplineType::BezierString:
		{
			FVector ComponentPosition = NewSplineComp->ConvertPosition(Position, CoordinateType, ECustomSplineCoordinateType::ComponentLocal);
			LatestNewPoint->SetRelativeLocation(ComponentPosition);
		}
			break;
		case ESplineType::ClampedBSpline:
		{
			LatestNewPoint = NewSplineComp->AddEndPoint(Position, true, CoordinateType);
		}
			break;
		}

		bSucceedReturn = (LatestNewPoint != nullptr);

		if (bSucceedReturn)
		{
			SourceSpline->SetCustomSelected(false);
		}
		return LatestNewPoint;
	}

	bSucceedReturn = false;
	return nullptr;
}

FVector ARuntimeSplineGraph::MovePoint(
	URuntimeCustomSplineBaseComponent* SourceSpline,
	URuntimeSplinePointBaseComponent* SourcePoint,
	const FVector& TargetPosition,
	ECustomSplineCoordinateType CoordinateType)
{
	FVector TargetSplineLocalPosition = FVector::ZeroVector;
	if (IsValid(SourceSpline) && !SourceSpline->IsBeingDestroyed() && IsValid(SourcePoint) && !SourcePoint->IsBeingDestroyed() && SourcePoint->SplinePointProxy.IsValid())
	{
		FSpatialControlPoint3& CPRef = *SourcePoint->SplinePointProxy.Pin().Get();
		TargetSplineLocalPosition = SourceSpline->ConvertPosition(TargetPosition, CoordinateType, ECustomSplineCoordinateType::SplineGraphLocal);
		SplineGraphProxy.AdjustCtrlPointPos(
			CPRef, TargetSplineLocalPosition, SourceSpline->GetSplineProxyWeakPtr(),
			1, SourcePoint->TangentFlag, 0);
		TMap<URuntimeCustomSplineBaseComponent*, int32> ClusterSplines;
		GetClusterSplinesWithoutSource(ClusterSplines, SourceSpline, true);
		for (TPair<URuntimeCustomSplineBaseComponent*, int32>& TargetSplinePair : ClusterSplines)
		{
			TargetSplinePair.Get<0>()->UpdateControlPointsLocation();
			TargetSplinePair.Get<0>()->UpdateTransformByCtrlPoint();
		}
		GetClusterSplinesWithoutSource(ClusterSplines, SourceSpline, false);
		for (TPair<URuntimeCustomSplineBaseComponent*, int32>& TargetSplinePair : ClusterSplines)
		{
			TargetSplinePair.Get<0>()->UpdateControlPointsLocation();
			TargetSplinePair.Get<0>()->UpdateTransformByCtrlPoint();
		}
		//SourcePoint->UpdateComponentLocationBySpline(); // Stack Overflow
	}

	return TargetSplineLocalPosition;
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

URuntimeCustomSplineBaseComponent* ARuntimeSplineGraph::CreateSplineActorInternal(TWeakPtr<FSpatialSplineBase3> SplineWeakPtr, URuntimeSplinePointBaseComponent** LatestNewPointPtr)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	AActor* NewActor = World->SpawnActor<AActor>(ActorWithSpline, GetActorTransform());
	if (!IsValid(NewActor))
	{
		return nullptr;
	}
	//if (ActorWithSpline.Get() == AActor::StaticClass()))
	if (!IsValid(NewActor->GetRootComponent()))
	{
		USceneComponent* NewRootComponent = NewObject<USceneComponent>(NewActor);
		NewActor->SetRootComponent(NewRootComponent);
		NewActor->AddInstanceComponent(NewRootComponent);
		NewRootComponent->RegisterComponent();
	}
	else
	{
		NewActor->AddInstanceComponent(NewActor->GetRootComponent());
	}

	URuntimeCustomSplineBaseComponent* NewSpline = NewObject<URuntimeCustomSplineBaseComponent>(NewActor, CustomSplineClass);
	NewSpline->AttachToComponent(NewActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	NewActor->AddInstanceComponent(NewSpline);
	NewSpline->RegisterComponent();
	NewSpline->SplineBaseWrapperProxy = SplineGraphProxy.GetSplineWrapper(SplineWeakPtr).Pin();
	VirtualAttachSplineComponent(NewSpline);

	if (SplineWeakPtr.IsValid())
	{
		FSpatialSplineBase3& SplineRef = *SplineWeakPtr.Pin().Get();
		if (SplineRef.GetCtrlPointNum() > 0)
		{
			TArray<TWeakPtr<FSpatialControlPoint3> > CtrlPointStructsWP;
			SplineRef.GetCtrlPointStructs(CtrlPointStructsWP);
			AddUnbindingPointsInternal(CtrlPointStructsWP, NewSpline, LatestNewPointPtr);
		}
	}

	NewSpline->SetCustomSelected(true);

	return NewSpline;
}

void ARuntimeSplineGraph::AddUnbindingPointsInternal(const TArray<TWeakPtr<FSpatialControlPoint3> >& CtrlPointStructsWP, URuntimeCustomSplineBaseComponent* NewSpline, URuntimeSplinePointBaseComponent** LatestNewPointPtr)
{
	if (!IsValid(NewSpline) || NewSpline->IsBeingDestroyed() || !NewSpline->SplineBaseWrapperProxy.IsValid() || !NewSpline->SplineBaseWrapperProxy.Get()->Spline.IsValid())
	{
		return;
	}
	FSpatialSplineBase3& SplineRef = *NewSpline->SplineBaseWrapperProxy.Get()->Spline.Get();
	for (const TWeakPtr<FSpatialControlPoint3>& WP : CtrlPointStructsWP)
	{
		if (WP.IsValid())
		{
			switch (SplineRef.GetType())
			{
			case ESplineType::BezierString:
			{
				//const auto& BeziersCP = MakeShared<TSplineTraitByType<ESplineType::BezierString>::FControlPointType&>(*WP.Pin().Get());
				TSharedRef<FSpatialControlPoint3> CPRef = WP.Pin().ToSharedRef();
				URuntimeSplinePointBaseComponent* LatestPoint = NewSpline->AddPointInternal(CPRef, 0);
				NewSpline->AddPointInternal(CPRef, -1);
				NewSpline->AddPointInternal(CPRef, 1);
				if (LatestNewPointPtr)
				{
					*LatestNewPointPtr = LatestPoint;
				}
			}
			break;
			case ESplineType::ClampedBSpline:
			{
				TSharedRef<FSpatialControlPoint3> CPRef = WP.Pin().ToSharedRef();
				URuntimeSplinePointBaseComponent* LatestPoint = NewSpline->AddPointInternal(CPRef, 0);
				if (LatestNewPointPtr)
				{
					*LatestNewPointPtr = LatestPoint;
				}
			}
			break;
			}
		}
	}
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
	
	bool bLocationChanged = (PropertyName == LocationName || MemberPropertyName == LocationName);
	bool bRotationChanged = (PropertyName == RotationName || MemberPropertyName == RotationName);
	bool bScaleChanged = (PropertyName == ScaleName || MemberPropertyName == ScaleName);
	
	if (bLocationChanged || bRotationChanged || bScaleChanged)
	{
		OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
	}
}

void USplineGraphRootComponent::PostEditComponentMove(bool bFinished)
{
	//if (bFinished)
	{
		OnUpdateTransform(EUpdateTransformFlags::None, ETeleportType::None);
	}

	Super::PostEditComponentMove(bFinished);
}

void ARuntimeSplineGraph::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//const static FName LocationName("RelativeLocation");
	//const static FName RotationName("RelativeRotation");
	//const static FName ScaleName("RelativeScale3D");

	Super::PostEditChangeProperty(PropertyChangedEvent);
	//const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	//const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();

	//bool bLocationChanged = (PropertyName == LocationName || MemberPropertyName == LocationName);
	//bool bRotationChanged = (PropertyName == RotationName || MemberPropertyName == RotationName);
	//bool bScaleChanged = (PropertyName == ScaleName || MemberPropertyName == ScaleName);

	//if (bLocationChanged || bRotationChanged || bScaleChanged)
	//{
	//	//SplineGraphRootComponent->SetRelativeTransform(GetActorTransform());
	//}
}
#endif
