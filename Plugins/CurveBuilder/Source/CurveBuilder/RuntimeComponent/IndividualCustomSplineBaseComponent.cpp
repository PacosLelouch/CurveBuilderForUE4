// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "IndividualCustomSplineBaseComponent.h"
#include "RuntimeCustomSplineBaseComponent.h"
#include "RuntimeSplinePointBaseComponent.h"
#include "SceneProxies/RuntimeCustomSplineSceneProxy.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/StaticMesh.h"
#include "Components/SplineMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

UIndividualCustomSplineBaseComponent::UIndividualCustomSplineBaseComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), OriginalMethod(CreationMethod)
{
	
	ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshFinder(TEXT("StaticMesh'/Engine/EditorLandscapeResources/SplineEditorMesh.SplineEditorMesh'"));
	if (StaticMeshFinder.Succeeded())
	{
		StaticMeshForSpline = StaticMeshFinder.Object;
	}
	//bWantsInitializeComponent = true;
	//SplineComponent = CreateDefaultSubobject<URuntimeCustomSplineBaseComponent>(TEXT("RuntimeCustomSplineBaseComponent_0"));
	//SplineComponent->SetupAttachment(this);
	//ConstructSplinePoints = TMap<URuntimeSplinePointBaseComponent*, FVector>
	//{
	//	{
	//		CreateDefaultSubobject<URuntimeSplinePointBaseComponent>(TEXT("RuntimeSplinePointBaseComponent_0")),
	//		FVector(0.f, 0.f, 0.f),
	//	},
	//	{
	//		CreateDefaultSubobject<URuntimeSplinePointBaseComponent>(TEXT("RuntimeSplinePointBaseComponent_1")),
	//		FVector(100.f, 0.f, 0.f),
	//	},
	//};
	//for (auto& Pair : ConstructSplinePoints)
	//{
	//	Pair.Get<0>()->SetupAttachment(this);
	//	SplinePoints.Add(Pair.Get<0>());
	//}
	////CreationMethod = EComponentCreationMethod::Instance;
	//SplineComponent = CreateDefaultSubobject<URuntimeCustomSplineBaseComponent>(TEXT("RuntimeCustomSplineBaseComponent"));
	////SplineComponent->CreationMethod = EComponentCreationMethod::Instance;
	//SplineComponent->SetupAttachment(this);
	//TMap<URuntimeSplinePointBaseComponent*, FVector> ConstructSplinePoints
	//{
	//	{
	//		CreateDefaultSubobject<URuntimeSplinePointBaseComponent>(TEXT("URuntimeSplinePointBaseComponent_0")),
	//		FVector(0.f, 0.f, 0.f),
	//	},
	//	{
	//		CreateDefaultSubobject<URuntimeSplinePointBaseComponent>(TEXT("URuntimeSplinePointBaseComponent_1")),
	//		FVector(100.f, 0.f, 0.f),
	//	},
	//};
	//for (auto& Pair : ConstructSplinePoints)
	//{
	//	//Pair.Get<0>()->CreationMethod = EComponentCreationMethod::Instance;
	//	Pair.Get<0>()->SetupAttachment(this);
	//	SplinePoints.Add(Pair.Get<0>());
	//}
	//ConstructIndividualSpline(
	//	ConstructSplinePoints,
	//	ECustomSplineCoordinateType::ComponentLocal,
	//	ERuntimeSplineType::ClampedBSpline);
}

void UIndividualCustomSplineBaseComponent::InitializeComponent()
{
	//CreationMethod = EComponentCreationMethod::Instance;
	////SplineComponent->CreationMethod = EComponentCreationMethod::Instance;
	////for (auto& Pair : ConstructSplinePoints)
	////{
	////	Pair.Get<0>()->CreationMethod = EComponentCreationMethod::Instance;
	////}
	//ConstructIndividualSpline(
	//	ConstructSplinePoints,
	//	ECustomSplineCoordinateType::ComponentLocal,
	//	ERuntimeSplineType::ClampedBSpline);
	//AActor* Owner = GetOwner();
	//UWorld* World = GetWorld();
	//if (IsValid(Owner) && !Owner->IsActorBeingDestroyed())
	//{
	//	CreationMethod = EComponentCreationMethod::Instance;
	//	//Owner->AddInstanceComponent(this);
	//	//RegisterComponent();
	//	//SplineComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	//	Owner->AddInstanceComponent(SplineComponent);
	//	////SplineComponent->RegisterComponent();
	//	for (auto* SplinePointComponent : SplinePoints)
	//	{
	//		//SplinePointComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	//		Owner->AddInstanceComponent(SplinePointComponent);
	//		//SplinePointComponent->RegisterComponent();
	//	}
	//}
}

void UIndividualCustomSplineBaseComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
}

void UIndividualCustomSplineBaseComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UIndividualCustomSplineBaseComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	bool bIsInstance = CreationMethod == EComponentCreationMethod::Instance;//Debug
	if (IsValid(Owner) && !Owner->IsActorBeingDestroyed())
	{
		//CreationMethod = EComponentCreationMethod::Instance;
		////Owner->AddInstanceComponent(this);
		////RegisterComponent();
		////SplineComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		//Owner->AddInstanceComponent(SplineComponent);
		//////SplineComponent->RegisterComponent();
		//for (auto* SplinePointComponent : SplinePoints)
		//{
		//	//SplinePointComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		//	Owner->AddInstanceComponent(SplinePointComponent);
		//	//SplinePointComponent->RegisterComponent();
		//}
	}
}

void UIndividualCustomSplineBaseComponent::OnRegister()
{
	Super::OnRegister();
	//AActor* Owner = GetOwner();
	//UWorld* World = GetWorld();
	//bool bIsInstance = CreationMethod == EComponentCreationMethod::Instance;//Debug
	//if (IsValid(Owner) && !Owner->IsActorBeingDestroyed())
	//{
	//	CreationMethod = EComponentCreationMethod::Instance;
	//	//Owner->AddInstanceComponent(this);
	//	//RegisterComponent();
	//	//SplineComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	//	Owner->AddInstanceComponent(SplineComponent);
	//	//SplineComponent->RegisterComponent();
	//	for (auto* SplinePointComponent : SplinePoints)
	//	{
	//		//SplinePointComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	//		Owner->AddInstanceComponent(SplinePointComponent);
	//		//SplinePointComponent->RegisterComponent();
	//	}
	//}
}

void UIndividualCustomSplineBaseComponent::OnUnregister()
{
	AActor* Owner = GetOwner();
	//if (IsValid(Owner) && !Owner->IsActorBeingDestroyed())
	//{
	//	Owner->ClearInstanceComponents(false);
	//}
	////SplineComponent->RegisterComponent();
	//for (auto It = SplinePoints.CreateIterator(); It; ++It)
	//{
	//	//SplinePointComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	//	(*It)->DestroyComponent();
	//	//SplinePointComponent->RegisterComponent();
	//}

	//SplineComponent->DestroyComponent();
	//CreationMethod = OriginalMethod;
	Super::OnUnregister();
}

#if WITH_EDITOR
void UIndividualCustomSplineBaseComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	const FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : FName();
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UIndividualCustomSplineBaseComponent, bAutoGenerateMesh))
	{
		if (IsValid(SplineComponent) && !SplineComponent->IsBeingDestroyed())
		{
			if (bAutoGenerateMesh)
			{
				SplineComponent->OnSplineUpdateHandle.AddDynamic(this, &UIndividualCustomSplineBaseComponent::GenerateSplineMeshesEvent);
			}
			else
			{
				SplineComponent->OnSplineUpdateHandle.RemoveDynamic(this, &UIndividualCustomSplineBaseComponent::GenerateSplineMeshesEvent);
			}
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(UIndividualCustomSplineBaseComponent, bDrawSplineInGame))
	{
		if (IsValid(SplineComponent) && !SplineComponent->IsBeingDestroyed())
		{
			SplineComponent->SetDrawInGame(bDrawSplineInGame);
			for (URuntimeSplinePointBaseComponent* PointComp : SplineComponent->PointComponents)
			{
				if (IsValid(PointComp) && !PointComp->IsBeingDestroyed())
				{
					PointComp->SetDrawInGame(bDrawSplineInGame);
				}
			}
		}
	}
}
#endif

void UIndividualCustomSplineBaseComponent::InitIndividualSpline()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (IsValid(Owner) && !Owner->IsActorBeingDestroyed() && IsValid(World))
	{
		USceneComponent* RootComponent = Owner->GetRootComponent();
		if (IsValid(RootComponent) && !RootComponent->IsBeingDestroyed())
		{
			Owner->AddInstanceComponent(RootComponent);
		}
		else
		{
			RootComponent = NewObject<USceneComponent>(Owner, TEXT("RootComponent"));
			Owner->SetRootComponent(RootComponent);
			Owner->AddInstanceComponent(RootComponent);
			RootComponent->RegisterComponent();
		}

		const TArray<USceneComponent*>& Children = GetAttachChildren();
		for (int32 i = Children.Num() - 1; i >= 0; --i)
		{
			USceneComponent* Comp = Children[i];
			if (Comp->IsA<URuntimeCustomSplineBaseComponent>() || Comp->IsA<URuntimeSplinePointBaseComponent>())
			{
				Comp->DestroyComponent();
			}
		}

		Owner->AddInstanceComponent(this);
		SplineComponent = NewObject<URuntimeCustomSplineBaseComponent>(this);
		SplineComponent->SplineBaseWrapperProxy = MakeShareable(new FSpatialSplineGraph3::FSplineWrapper());
		switch (ARuntimeSplineGraph::GetInternalSplineType(InitSplineType))
		{
		case ESplineType::Unknown:
		case ESplineType::ClampedBSpline:
			SplineComponent->SplineBaseWrapperProxy.Get()->Spline = MakeShareable(new TSplineTraitByType<ESplineType::ClampedBSpline>::FSplineType());
			break;
		case ESplineType::BezierString:
			SplineComponent->SplineBaseWrapperProxy.Get()->Spline = MakeShareable(new TSplineTraitByType<ESplineType::BezierString>::FSplineType());
			break;
		}
		SplineComponent->bDrawInGame = bDrawSplineInGame;
		SplineComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		Owner->AddInstanceComponent(SplineComponent);
		SplineComponent->SetCustomSelected(true);
		SplineComponent->RegisterComponent();
		for (const FVector& Pos : InitSplinePointPos)
		{
			SplineComponent->AddEndPoint(Pos, true, InitCoordinateType);
		}
	}
	if (bAutoGenerateMesh && 
		!SplineComponent->OnSplineUpdateHandle.Contains(this, UE4Delegates_Private::GetTrimmedMemberFunctionName(
			TEXT("&UIndividualCustomSplineBaseComponent::GenerateSplineMeshesEvent"))))
	{
		SplineComponent->OnSplineUpdateHandle.AddDynamic(this, &UIndividualCustomSplineBaseComponent::GenerateSplineMeshesEvent);
	}
	//Modify();
}

void UIndividualCustomSplineBaseComponent::GenerateSplineMeshes()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (IsValid(StaticMeshForSpline) && IsValid(SplineComponent) && !SplineComponent->IsBeingDestroyed()
		&& IsValid(Owner) && !Owner->IsActorBeingDestroyed() && IsValid(World))
	{
		for (auto It = SplineMeshes.CreateIterator(); It; ++It)
		{
			if ((*It)->IsA<USplineMeshComponent>())
			{
				Owner->RemoveInstanceComponent(*It);
				(*It)->DestroyComponent();
			}
		}
		TArray<FVector> Positions, ArriveTangents, LeaveTangents;
		SplineComponent->GetHermiteForms(Positions, ArriveTangents, LeaveTangents, ECustomSplineCoordinateType::World);

		SplineMeshes.Empty(Positions.Num() - 1);
		for (int32 i = 0; i + 1 < Positions.Num(); ++i)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->Mobility = Mobility;
			SplineMesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(StaticMeshForSpline);
			SplineMesh->SetWorldLocation(Positions[i]);
			Owner->AddInstanceComponent(SplineMesh);
			SplineMesh->RegisterComponent(); 
			SplineMesh->SetStartAndEnd(FVector::ZeroVector, LeaveTangents[i], Positions[i + 1] - Positions[i], ArriveTangents[i + 1], true);

			SplineMeshes.Add(SplineMesh);
		}
	}
}

void UIndividualCustomSplineBaseComponent::GenerateSplineMeshesEvent(URuntimeCustomSplineBaseComponent* InSpline)
{
	if (InSpline == SplineComponent)
	{
		GenerateSplineMeshes();
	}
}

void UIndividualCustomSplineBaseComponent::ConstructIndividualSpline(
	const TMap<URuntimeSplinePointBaseComponent*, FVector>& SplinePointsCompToPos,
	ECustomSplineCoordinateType CoordinateType,
	ERuntimeSplineType SplineType)
{
	SplineComponent->bCustomSelected = true;
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
			PointComponent->SetVisibility(SplineComponent->bCustomSelected);

			//UpdateTransformByCtrlPoint(); // Move to Point OnComponentCreated
		}
	}
}
