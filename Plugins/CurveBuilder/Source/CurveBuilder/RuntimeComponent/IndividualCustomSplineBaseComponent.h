// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "RuntimeSplinePrimitiveComponent.h"
#include "../Compute/Splines/SplineGraph.h"
#include "IndividualCustomSplineBaseComponent.generated.h"

class UStaticMesh;
class USplineMeshComponent;

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API UIndividualCustomSplineBaseComponent : public USceneComponent//, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()
public:
	UIndividualCustomSplineBaseComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeComponent() override;

	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual void OnAttachmentChanged() override;

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "RuntimeCustomSpline|Individual")
	void InitIndividualSpline();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "RuntimeCustomSpline|Individual")
	void GenerateSplineMeshes();

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Individual")
	void GenerateSplineMeshesEvent(URuntimeCustomSplineBaseComponent* InSpline);

	// Do not use this if you want to make spline graph
	//UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Individual", meta = (AutoCreateRefTerm = "SplinePointPositions"))
	void ConstructIndividualSpline(
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Individual")
	TArray<FVector> InitSplinePointPos { FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f) };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Individual")
	ECustomSplineCoordinateType InitCoordinateType = ECustomSplineCoordinateType::ComponentLocal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Individual")
	ERuntimeSplineType InitSplineType = ERuntimeSplineType::ClampedBSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Individual")
	UStaticMesh* StaticMeshForSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Individual")
	bool bAutoGenerateMesh = false;


	UPROPERTY(BlueprintReadWrite, Category = "RuntimeCustomSpline")
	URuntimeCustomSplineBaseComponent* SplineComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline")
	TSet<URuntimeSplinePointBaseComponent*> SplinePoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline")
	TSet<USplineMeshComponent*> SplineMeshes;

private:
	EComponentCreationMethod OriginalMethod;
	TMap<URuntimeSplinePointBaseComponent*, FVector> ConstructSplinePoints;
};