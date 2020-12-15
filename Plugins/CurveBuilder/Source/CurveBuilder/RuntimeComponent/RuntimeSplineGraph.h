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
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline")
	void VirtualAttachSplineComponent(URuntimeCustomSplineBaseComponent* SplineComponent);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline")
	void GetOwningSplines(TArray<URuntimeCustomSplineBaseComponent*>& Splines);

public:
	URuntimeCustomSplineBaseComponent* GetSplineComponentBySplineWeakPtr(TWeakPtr<FSpatialSplineGraph3::FSplineType> SplineWeakPtr);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuntimeCustomSpline|Component")
	USplineGraphRootComponent* SplineGraphRootComponent = nullptr;

public:
	FSpatialSplineGraph3 SplineGraphProxy;
	TMap<TSharedPtr<FSpatialSplineGraph3::FSplineWrapper>, URuntimeCustomSplineBaseComponent*> SplineComponentMap;
};