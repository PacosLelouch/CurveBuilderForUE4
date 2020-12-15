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

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Mobile))
class CURVEBUILDER_API URuntimeSplinePointBaseComponent : public URuntimeSplinePrimitiveComponent
{
	GENERATED_BODY()
public:
	URuntimeSplinePointBaseComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual void UpdateCollision() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditComponentMove(bool bFinished) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|DrawInfo")
	void SetSelected(bool bValue);

public:
	void MoveSplinePointInternal();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	FVector SplineLocalPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	int32 TangentFlag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	bool bSelected = false;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	UPROPERTY(SimpleDisplay, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
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
};
