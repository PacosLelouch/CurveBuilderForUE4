// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeSplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeSplinePointBaseComponent.h"
#include "GameFramework/PlayerController.h"

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

bool ARuntimeSplineGraph::TraceSplinePoint(URuntimeSplinePointBaseComponent*& OutTracedComponent, APlayerController* PlayerController, const FVector2D& MousePosition)
{
	OutTracedComponent = nullptr;
	if (!PlayerController)
	{
		return false;
	}
	TArray<TPair<FVector2D, URuntimeSplinePointBaseComponent*> > PointArray;
	for (auto& SplinePair : SplineComponentMap)
	{
		if (!IsValid(SplinePair.Get<1>()) || SplinePair.Get<1>()->IsBeingDestroyed())
		{
			continue;
		}

		for (auto* PointComp : SplinePair.Get<1>()->PointComponents)
		{
			auto* CastedPointComp = PointComp;
			{
				if (!CastedPointComp->IsVisible())
				{
					continue;
				}
				FVector ScreenLocation;
				bool bSucceed = PlayerController->ProjectWorldLocationToScreenWithDistance(CastedPointComp->GetComponentLocation(), ScreenLocation, true);
				if (!bSucceed)
				{
					continue;
				}

				if (ScreenLocation.Z < 0.f)
				{
					continue;
				}
				FVector2D Key((FVector2D(ScreenLocation) - MousePosition).SizeSquared(), ScreenLocation.Z);
				PointArray.Add(MakeTuple(Key, CastedPointComp));
			}
		}
	}

	if (PointArray.Num() == 0)
	{
		return false;
	}

	// Find Point?
	FVector2D MinDistSqrAndDepth = PointArray[0].Get<0>();
	auto* TargetSplinePoint = PointArray[0].Get<1>();

	for (int32 i = 1; i < PointArray.Num(); ++i)
	{
		const FVector2D& CurDistSqrAndDepth = PointArray[i].Get<0>();
		if ((FMath::IsNearlyEqual(CurDistSqrAndDepth.X, MinDistSqrAndDepth.X) && CurDistSqrAndDepth.Y < MinDistSqrAndDepth.Y)
			|| (CurDistSqrAndDepth.X < MinDistSqrAndDepth.X))
		{
			MinDistSqrAndDepth = CurDistSqrAndDepth;
			TargetSplinePoint = PointArray[i].Get<1>();
		}
	}

	if (MinDistSqrAndDepth.X * 4.f <= FMath::Square(TargetSplinePoint->CollisionDiameter))
	{
		OutTracedComponent = TargetSplinePoint;
		return true;
	}

	return false;
}

bool ARuntimeSplineGraph::TraceSpline(URuntimeCustomSplineBaseComponent*& OutTracedComponent, float& OutTracedParam, FVector& OutTracedWorldPos, APlayerController* PlayerController, const FVector2D& MousePosition)
{
	OutTracedComponent = nullptr;
	OutTracedParam = -1.f;
	OutTracedWorldPos = FVector::ZeroVector;
	if (!PlayerController)
	{
		return false;
	}
	TMap<URuntimeCustomSplineBaseComponent*, TSharedPtr<FSpatialSplineBase3> > ScreenSpaceSplineMap;
	TMap<URuntimeCustomSplineBaseComponent*, TSharedPtr<FSpatialSplineBase3> > ScreenSpaceSplinePlanarMap;
	TMap<URuntimeCustomSplineBaseComponent*, FBox> ScreenSpaceBoxMap;
	ScreenSpaceSplineMap.Reserve(SplineComponentMap.Num());
	ScreenSpaceSplinePlanarMap.Reserve(SplineComponentMap.Num());
	ScreenSpaceBoxMap.Reserve(SplineComponentMap.Num());
	for (auto& SplinePair : SplineComponentMap)
	{
		auto* SplineComp = SplinePair.Get<1>();
		//URoadEditorSplineComponent* SplineComp = Cast<URoadEditorSplineComponent>(SplinePair.Get<1>());
		if (!IsValid(SplineComp) || SplineComp->IsBeingDestroyed() || !SplineComp->IsVisible())// || SplineComp->bIsOffset)
		{
			continue;
		}

		FBox& NewBox = ScreenSpaceBoxMap.Add(SplineComp, FBox(EForceInit::ForceInit));

		auto* SplineProxy = SplineComp->GetSplineProxy();
		TArray<TWeakPtr<FSpatialControlPoint3> > CPStructs;
		SplineProxy->GetCtrlPointStructs(CPStructs);

		FTransform SplineLocalToWorld = SplineComp->GetSplineLocalToWorldTransform();

		switch (SplineProxy->GetType())
		{
		case ESplineType::ClampedBSpline:
		{
			TArray<FVector4> CPs;
			TArray<FVector4> CPsPlanar;
			TArray<double> Intervals;
			CPs.Reserve(CPStructs.Num());
			CPsPlanar.Reserve(CPStructs.Num());
			static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType*>(SplineProxy)->GetKnotIntervals(Intervals);
			for (auto& CPStructWeakPtr : CPStructs)
			{
				FVector ScreenLocation = FVector::ZeroVector;
				auto* CPStruct = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FControlPointType*>(CPStructWeakPtr.Pin().Get());
				PlayerController->ProjectWorldLocationToScreenWithDistance(SplineLocalToWorld.TransformPosition(TVecLib<4>::Projection(CPStruct->Pos)), ScreenLocation, true);
				CPs.Add(FVector4(ScreenLocation, 1.f));
				CPsPlanar.Add(FVector4(ScreenLocation.X, ScreenLocation.Y, 0.f, 1.f));
				NewBox += ScreenLocation;
			}
			auto* NewSplinePtr = new TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType();
			NewSplinePtr->Reset(CPs, Intervals);
			ScreenSpaceSplineMap.Add(SplineComp, MakeShareable(NewSplinePtr));

			auto* NewSplinePlanarPtr = new TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType();
			NewSplinePlanarPtr->Reset(CPsPlanar, Intervals);
			ScreenSpaceSplinePlanarMap.Add(SplineComp, MakeShareable(NewSplinePlanarPtr));
		}
		break;
		case ESplineType::BezierString:
		{
			TArray<FVector4> CPs;
			TArray<FVector4> CPsPlanar;
			TArray<FVector4> PCPs;
			TArray<FVector4> PCPsPlanar;
			TArray<FVector4> NCPs;
			TArray<FVector4> NCPsPlanar;
			TArray<double> Params;
			TArray<EEndPointContinuity> Continuities;
			CPs.Reserve(CPStructs.Num());
			CPsPlanar.Reserve(CPStructs.Num());
			PCPs.Reserve(CPStructs.Num());
			PCPsPlanar.Reserve(CPStructs.Num());
			NCPs.Reserve(CPStructs.Num());
			NCPsPlanar.Reserve(CPStructs.Num());
			Params.Reserve(CPStructs.Num());
			Continuities.Reserve(CPStructs.Num());
			for (auto& CPStructWeakPtr : CPStructs)
			{
				FVector ScreenLocation = FVector::ZeroVector;
				auto* CPStruct = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FControlPointType*>(CPStructWeakPtr.Pin().Get());
				PlayerController->ProjectWorldLocationToScreenWithDistance(SplineLocalToWorld.TransformPosition(TVecLib<4>::Projection(CPStruct->Pos)), ScreenLocation, true);
				CPs.Add(FVector4(ScreenLocation, 1.f));
				CPsPlanar.Add(FVector4(ScreenLocation.X, ScreenLocation.Y, 0.f, 1.f));
				NewBox += ScreenLocation;

				PlayerController->ProjectWorldLocationToScreenWithDistance(SplineLocalToWorld.TransformPosition(TVecLib<4>::Projection(CPStruct->PrevCtrlPointPos)), ScreenLocation, true);
				PCPs.Add(FVector4(ScreenLocation, 1.f));
				PCPsPlanar.Add(FVector4(ScreenLocation.X, ScreenLocation.Y, 0.f, 1.f));
				NewBox += ScreenLocation;

				PlayerController->ProjectWorldLocationToScreenWithDistance(SplineLocalToWorld.TransformPosition(TVecLib<4>::Projection(CPStruct->NextCtrlPointPos)), ScreenLocation, true);
				NCPs.Add(FVector4(ScreenLocation, 1.f));
				NCPsPlanar.Add(FVector4(ScreenLocation.X, ScreenLocation.Y, 0.f, 1.f));
				NewBox += ScreenLocation;

				Params.Add(CPStruct->Param);
				Continuities.Add(CPStruct->Continuity);
			}
			auto* NewSplinePtr = new TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType();
			NewSplinePtr->Reset(CPs, PCPs, NCPs, Params, Continuities);
			ScreenSpaceSplineMap.Add(SplineComp, MakeShareable(NewSplinePtr));

			auto* NewSplinePlanarPtr = new TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType();
			NewSplinePlanarPtr->Reset(CPsPlanar, PCPsPlanar, NCPsPlanar, Params, Continuities);
			ScreenSpaceSplinePlanarMap.Add(SplineComp, MakeShareable(NewSplinePlanarPtr));
		}
		break;
		}
	}

	float NearestZ = TNumericLimits<float>::Max();
	for (auto& ScreenSpaceSplinePair : ScreenSpaceSplineMap)
	{
		auto* SplineComp = ScreenSpaceSplinePair.Get<0>();
		const FBox& ScreenSpaceBox = ScreenSpaceBoxMap[SplineComp];
		if (ScreenSpaceBox.Max.Z >= 0.f && ScreenSpaceBox.IsInsideXY(FVector(MousePosition, 0.f)))
		{
			double Param = -1.;
			auto* SplinePlanarProxy = ScreenSpaceSplinePlanarMap[SplineComp].Get();
			if (SplinePlanarProxy->FindParamByPosition(Param, FVector(MousePosition, 0.f), FMath::Square(SplineComp->CollisionSegWidth)))
			{
				auto* SplineProxy = ScreenSpaceSplinePair.Get<1>().Get();
				FVector ScreenSpaceWithDist = SplineProxy->GetPosition(Param);
				if (ScreenSpaceWithDist.Z < NearestZ)
				{
					NearestZ = ScreenSpaceWithDist.Z;
					OutTracedParam = Param;
					OutTracedComponent = SplineComp;
				}
			}
		}
	}
	if (OutTracedComponent)
	{
		OutTracedWorldPos = OutTracedComponent->ConvertPosition(
			OutTracedComponent->GetSplineProxy()->GetPosition(OutTracedParam),
			ECustomSplineCoordinateType::SplineGraphLocal,
			ECustomSplineCoordinateType::World);
		return true;
	}
	return false;
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
	TArray<TWeakPtr<FSpatialSplineBase3::FControlPointType> > NewSrcCtrlPoints, NewTarCtrlPoints;
	EXEC_WITH_THREAD_MUTEX_LOCK(Source->RenderMuteX,
		TWeakPtr<FSpatialSplineBase3> ReturnSplineWeakPtr = SplineGraphProxy.ConnectAndFill(
			Source->SplineBaseWrapperProxy.Get()->Spline, Target->SplineBaseWrapperProxy.Get()->Spline,
			bSourceForward ? EContactType::End : EContactType::Start,
			bTargetForward ? EContactType::End : EContactType::Start,
			bFillInSource,
			&NewSrcCtrlPoints,
			&NewTarCtrlPoints);
	);

	if (!ReturnSplineWeakPtr.IsValid())
	{
		return nullptr;
	}

	AddUnbindingPointsInternal(NewSrcCtrlPoints, Source);
	AddUnbindingPointsInternal(NewTarCtrlPoints, Target);

	if (!bFillInSource)
	{
		auto* NewSplineComponent = CreateSplineActorInternal(ReturnSplineWeakPtr);
		return NewSplineComponent;
	}
	//else // Fill in source. Need to bind new points.
	//{
	//	TArray<TWeakPtr<FSpatialControlPoint3> > NewCtrlPointStructs;
	//	auto* SourceSpline = ReturnSplineWeakPtr.Pin().Get();
	//	switch (SourceSpline->GetType())
	//	{
	//	case ESplineType::ClampedBSpline:
	//	{
	//		auto* SBSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType*>(SourceSpline);
	//		TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType::FPointNode* FirstNode = nullptr;
	//		TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType::FPointNode* SecondNode = nullptr;
	//		if (bSourceForward)
	//		{
	//			SecondNode = SBSpline->LastNode();
	//			FirstNode = SecondNode->GetPrevNode();
	//		}
	//		else
	//		{
	//			SecondNode = SBSpline->FirstNode();
	//			FirstNode = SecondNode->GetNextNode();
	//		}
	//		NewCtrlPointStructs.Add(FirstNode->GetValue());
	//		NewCtrlPointStructs.Add(SecondNode->GetValue());
	//	}
	//		break;
	//	case ESplineType::BezierString:
	//	{
	//		auto* SBeziers = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType*>(SourceSpline);
	//		TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType::FPointNode* FirstNode = nullptr;
	//		if (bSourceForward)
	//		{
	//			FirstNode = SBeziers->LastNode();
	//		}
	//		else
	//		{
	//			FirstNode = SBeziers->FirstNode();
	//		}
	//		NewCtrlPointStructs.Add(FirstNode->GetValue());
	//	}
	//		break;
	//	}
	//	AddUnbindingPointsInternal(NewCtrlPointStructs, Source);
	//}
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
		SplineComponentMap.Remove(SplineToDelete->SplineBaseWrapperProxy);
		SplineGraphProxy.DeleteSpline(SplineToDelete->SplineBaseWrapperProxy.Get()->Spline);
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

FVector ARuntimeSplineGraph::MovePointInternal(
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

	if (bAutoSelectNewSpline)
	{
		NewSpline->SetCustomSelected(true);
	}

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
