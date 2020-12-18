// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
//#include "Components/BoxComponent.h"
//#include "Components/PrimitiveComponent.h"
#include "RuntimeSplinePrimitiveComponent.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeSplinePointBaseComponent.generated.h"

class URuntimeCustomSplineBaseComponent;

UCLASS(BlueprintType, Blueprintable, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API URuntimeSplinePointBaseComponent : public URuntimeSplinePrimitiveComponent
{
	GENERATED_BODY()
public:
	URuntimeSplinePointBaseComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void OnVisibilityChanged() override;

	virtual void OnComponentCreated() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual void UpdateCollision() override;

	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags = EUpdateTransformFlags::None, ETeleportType Teleport = ETeleportType::None) override;

	virtual bool MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit = NULL, EMoveComponentFlags MoveFlags = MOVECOMP_NoFlags, ETeleportType Teleport = ETeleportType::None) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditComponentMove(bool bFinished) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|DrawInfo")
	void SetSelected(bool bValue);

	// Deprecated function. Please use SetRelativeLocation to move point!
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void MoveTo_Deprecated(const FVector& Position, ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void UpdateComponentLocationBySpline();

public:
	void MoveSplinePointInternal();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	FVector SplineLocalPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	int32 TangentFlag = 0;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	//URuntimeSplinePointBaseComponent* PrevPointForTangent = nullptr;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	//URuntimeSplinePointBaseComponent* NextPointForTangent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	bool bSelected = false;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	UPROPERTY(AdvancedDisplay, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	URuntimeCustomSplineBaseComponent* ParentSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	float CollisionDiameter = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	int32 DrawPointSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor CtrlPointColor = FLinearColor(0.4f, 0.8f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor SelectedCtrlPointColor = FLinearColor(0.9f, 0.3f, 0.1f);

public:
	TWeakPtr<FSpatialControlPoint3> SplinePointProxy;

private:
	bool bIsDuplicated = false;
};
