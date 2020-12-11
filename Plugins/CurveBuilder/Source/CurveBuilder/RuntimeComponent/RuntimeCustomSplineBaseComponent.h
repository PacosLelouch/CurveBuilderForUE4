// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "Components/PrimitiveComponent.h"
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.generated.h"

using FSpatialSplineBase3 = typename TSplineBase<3, 3>;

template<int32 Dim = 3, int32 Degree = 3>
struct TRuntimeSplineDrawInfo;

using FSpatial3DrawInfo = typename TRuntimeSplineDrawInfo<3, 3>;

class URuntimeSplinePointBaseComponent;

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Rendering, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API URuntimeCustomSplineBaseComponent : public UPrimitiveComponent//, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FMatrix GetRenderMatrix() const override;

	virtual UBodySetup* GetBodySetup() override { return BodySetup; }

	virtual void OnCreatePhysicsState() override;

	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|DrawInfo")
	void SetSelected(bool bValue);

public:

	FSpatialSplineGraph3::FSplineType* GetSplineProxy() const;

	template<typename T>
	T* GetSplineProxyInternal() const
	{
		if (SplineBaseWrapperProxy.Get() && SplineBaseWrapperProxy.Get()->Spline.Get())
		{
			return static_cast<T*>(SplineBaseWrapperProxy.Get()->Spline.Get());
		}
		return nullptr;
	}

	FTransform GetSplineLocalToWorldTransform() const;
	FMatrix GetSplineLocalToWorldMatrix() const;
	FMatrix GetSplineWorldToLocalMatrix() const;

	void UpdateTransformByCtrlPoint();

	void CreateBodySetup();

	void UpdateCollision();

	// Static function of draw spline curves.
	//template<int32 Dim = 3, int32 Degree = 3>
	//static void DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const TRuntimeSplineDrawInfo<Dim, Degree>& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup);
	
	// Static function of draw spline curves.
	static void DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	bool bSelected = false;

	// Be parameter length or curve length?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float SegLength = 0.05f;//10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float Thickness = 2.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float DepthBias = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	int32 PointSize = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor CurveColor = FLinearColor(0.1f, 0.6f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor CtrlSegColor = FLinearColor(0.2f, 1.f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor CtrlPointColor = FLinearColor(0.4f, 0.8f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor SelectedCtrlPointColor = FLinearColor(0.9f, 0.3f, 0.1f);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline")
	ARuntimeSplineGraph* ParentGraph = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline")
	TArray<URuntimeSplinePointBaseComponent*> PointComponents;

	UPROPERTY(Instanced)
	UBodySetup* BodySetup;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	float CollisionSegLength = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	float CollisionSegWidth = 8.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	bool bDrawDebugCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	FLinearColor DebugCollisionColor = FLinearColor(0.2f, 0.8f, 0.2f);

public:
	TSharedPtr<FSpatialSplineGraph3::FSplineWrapper> SplineBaseWrapperProxy;

};
