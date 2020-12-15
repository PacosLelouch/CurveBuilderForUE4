// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "Components/PrimitiveComponent.h"
#include "RuntimeSplinePrimitiveComponent.generated.h"

UCLASS(BlueprintType, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API URuntimeSplinePrimitiveComponent : public UPrimitiveComponent//, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()
public:
	URuntimeSplinePrimitiveComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FMatrix GetRenderMatrix() const override;

	virtual UBodySetup* GetBodySetup() override { return BodySetup; }

	virtual void OnCreatePhysicsState() override;

	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	virtual void SetDrawDebugCollision(bool bValue);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	FTransform GetSplineLocalToWorldTransform() const;
	FTransform GetWorldToSplineLocalTransform() const;
	FMatrix GetSplineLocalToWorldMatrix() const;
	FMatrix GetWorldToSplineLocalMatrix() const;
	FTransform GetSplineLocalToComponentLocalTransform() const;
	FTransform GetComponentLocalToSplineLocalTransform() const;
	FTransform GetSplineLocalToParentComponentTransform() const;
	FTransform GetParentComponentToSplineLocalTransform() const;

	virtual void CreateBodySetup();

	virtual void UpdateCollision();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	ARuntimeSplineGraph* ParentGraph = nullptr;

	UPROPERTY(Instanced)
	UBodySetup* BodySetup = nullptr;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float DepthBias = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionDebugInfo")
	float DebugCollisionLineWidth = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionDebugInfo")
	bool bDrawDebugCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionDebugInfo")
	FLinearColor DebugCollisionColor = FLinearColor(0.2f, 0.7f, 0.2f);

public:
	static ECollisionTraceFlag SpCompCollisionTraceFlag;
};
