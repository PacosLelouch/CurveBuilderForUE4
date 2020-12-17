// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "RuntimeSplinePrimitiveComponent.h"
#include "../Compute/Splines/SplineGraph.h"
#include "IndividualCustomSplineBaseComponent.generated.h"

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API UIndividualCustomSplineBaseComponent : public USceneComponent//, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()
public:
	UIndividualCustomSplineBaseComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//virtual void OnComponentCreated() override;
	//virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	// Do not use this if you want to make spline graph
	//UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Individual", meta = (AutoCreateRefTerm = "SplinePointPositions"))
	void CreateIndividualSpline(
		const TMap<URuntimeSplinePointBaseComponent*, FVector>& SplinePointsCompToPos,
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::ComponentLocal,
		ERuntimeSplineType SplineType = ERuntimeSplineType::ClampedBSpline);

	//UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void ConstructEndPoint(
		URuntimeSplinePointBaseComponent* PointComponent,
		const FVector& Position,
		bool bAtLast = true,
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	void ConstructPointInternal(
		URuntimeSplinePointBaseComponent* PointComponent, 
		const TSharedRef<FSpatialControlPoint3>& PointRef, int32 TangentFlag = 0);

public:
	UPROPERTY(Instanced, BlueprintReadWrite, Category = "RuntimeSplineComponent")
	URuntimeCustomSplineBaseComponent* SplineComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeSplineComponent")
	TSet<URuntimeSplinePointBaseComponent*> SplinePoints;

private:
	EComponentCreationMethod OriginalMethod;
};