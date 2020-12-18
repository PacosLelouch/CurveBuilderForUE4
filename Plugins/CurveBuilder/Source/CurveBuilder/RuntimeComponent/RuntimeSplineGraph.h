// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeSplineGraph.generated.h"

using FSpatialSplineGraph3 = typename TSplineGraph<3, 3>;
using FSpatialSplineBase3 = typename TSplineBase<3, 3>;
using FSpatialControlPoint3 = typename TSplineBaseControlPoint<3, 3>;

class URuntimeCustomSplineBaseComponent;

UENUM(BlueprintType)
enum class ECustomSplineCoordinateType : uint8
{
	SplineGraphLocal,
	ComponentLocal,
	World,
};

UENUM(BlueprintType)
enum class ERuntimeSplineType : uint8
{
	Unknown,
	ClampedBSpline,
	BezierString,
};

UCLASS()
class CURVEBUILDER_API USplineGraphRootComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditComponentMove(bool bFinished) override;
#endif

};

UCLASS(Blueprintable)
class CURVEBUILDER_API ARuntimeSplineGraph : public AActor
{
	GENERATED_BODY()
public:
	ARuntimeSplineGraph(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void Destroyed() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void VirtualAttachSplineComponent(URuntimeCustomSplineBaseComponent* SplineComponent);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Query")
	void GetOwningSplines(TArray<URuntimeCustomSplineBaseComponent*>& Splines);

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	bool CheckSplineHasConnection(URuntimeCustomSplineBaseComponent* SplineComponent, bool bAtLast = true);

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	void GetAdjacentSplines(TArray<URuntimeCustomSplineBaseComponent*>& OutAdjacentSplines, URuntimeCustomSplineBaseComponent* SourceSpline, bool bForward = true);

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	void GetClusterSplinesWithoutSource(TMap<URuntimeCustomSplineBaseComponent*, int32>& OutClusterSplinesWithDistance, URuntimeCustomSplineBaseComponent* SourceSpline, bool bForward = true);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void ClearAllSplines();

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	URuntimeCustomSplineBaseComponent* CreateNewActorWithEmptySpline(
		ERuntimeSplineType SplineTypeToCreate = ERuntimeSplineType::ClampedBSpline);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	URuntimeSplinePointBaseComponent* AddEndPoint(
		const FVector& Position, 
		URuntimeCustomSplineBaseComponent* ToSpline = nullptr, 
		bool bAtLast = true, 
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal, 
		ERuntimeSplineType SplineTypeToCreate = ERuntimeSplineType::ClampedBSpline);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	URuntimeSplinePointBaseComponent* InsertPoint(
		const FVector& Position,
		bool& bSucceedReturn,
		URuntimeCustomSplineBaseComponent* ToSpline = nullptr,
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	URuntimeCustomSplineBaseComponent* ExtendNewSplineWithContinuity(
		bool& bSucceedReturn,
		URuntimeCustomSplineBaseComponent* SourceSpline = nullptr,
		bool bAtLast = true,
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	URuntimeSplinePointBaseComponent* ExtendNewSplineAndNewPoint(
		const FVector& Position,
		bool& bSucceedReturn,
		URuntimeCustomSplineBaseComponent* SourceSpline = nullptr,
		bool bAtLast = true,
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	FVector MovePoint(
		URuntimeCustomSplineBaseComponent* SourceSpline, 
		URuntimeSplinePointBaseComponent* SourcePoint,
		const FVector& TargetPosition,
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);


public:

	URuntimeCustomSplineBaseComponent* GetSplineComponentBySplineWeakPtr(TWeakPtr<FSpatialSplineBase3> SplineWeakPtr);

	URuntimeCustomSplineBaseComponent* CreateSplineActorInternal(TWeakPtr<FSpatialSplineBase3> SplineWeakPtr, URuntimeSplinePointBaseComponent** LatestNewPointPtr = nullptr);

	void AddUnbindingPointsInternal(const TArray<TWeakPtr<FSpatialControlPoint3> >& CtrlPointStructsWP, URuntimeCustomSplineBaseComponent* NewSpline, URuntimeSplinePointBaseComponent** LatestNewPointPtr = nullptr);

	static ESplineType GetInternalSplineType(ERuntimeSplineType SplineType)
	{
		return static_cast<ESplineType>(SplineType);
	}

	static ERuntimeSplineType GetExternalSplineType(ESplineType InternalSplineType)
	{
		return static_cast<ERuntimeSplineType>(InternalSplineType);
	}

public:
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, BlueprintReadOnly, Category = "RuntimeCustomSpline|Component")
	USplineGraphRootComponent* SplineGraphRootComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuntimeCustomSpline|Component")
	TArray<URuntimeCustomSplineBaseComponent*> SelectedSplines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Settings")
	TSubclassOf<AActor> ActorWithSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Settings")
	TSubclassOf<URuntimeCustomSplineBaseComponent> CustomSplineClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Settings")
	TSubclassOf<URuntimeSplinePointBaseComponent> CustomSplinePointClass;

public:
	FSpatialSplineGraph3 SplineGraphProxy;
	TMap<TSharedPtr<FSpatialSplineGraph3::FSplineWrapper>, URuntimeCustomSplineBaseComponent*> SplineComponentMap;
};