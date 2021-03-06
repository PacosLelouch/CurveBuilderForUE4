// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "RuntimeSplinePrimitiveComponent.h"
//#include "Interfaces/Interface_CollisionDataProvider.h"
#include "../Compute/Splines/SplineGraph.h"
#include "RuntimeCustomSplineBaseComponent.generated.h"

class CURVEBUILDER_API FRuntimeSplineCommands : public TCommands<FRuntimeSplineCommands>
{
public:
	FRuntimeSplineCommands();

	virtual void RegisterCommands() override;

public:
	/** Insert control point here */
	TSharedPtr<FUICommandInfo> InsertControlPoint;

	/** Add new spline at end */
	TSharedPtr<FUICommandInfo> AddNewSplineAtEnd;

	/** Add new spline at start */
	TSharedPtr<FUICommandInfo> AddNewSplineAtStart;

	/** Reverse spline */
	TSharedPtr<FUICommandInfo> ReverseSpline;
};

class CURVEBUILDER_API FRuntimeSplineCommandHelper : public FRuntimeSplineCommandHelperBase
{
public:
	FRuntimeSplineCommandHelper(class URuntimeCustomSplineBaseComponent* Component = nullptr)
		: FRuntimeSplineCommandHelperBase()
		, ComponentWeakPtr(Component)
	{
		FRuntimeSplineCommands::Register();
	}

	virtual void CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;

	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = false) override;

	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;

protected:
	void OnInsertControlPoint();
	bool CanInsertControlPoint() const;

	void OnAddNewSplineAdj(bool bAtLast);
	bool CanAddNewSplineAdj() const;

	//void OnAddNewSplineAtStart();
	//bool CanAddNewSplineAtStart() const;

	void OnReverseSpline();
	bool CanReverseSpline() const;

public:
	virtual void MapActions() override;

	virtual void GenerateContextMenuSections(FMenuBuilder& InMenuBuilder) const override;

protected:
	virtual void GenerateContextMenu_AddNewSpline(FMenuBuilder& InMenuBuilder) const;

public:
	TWeakObjectPtr<URuntimeCustomSplineBaseComponent> ComponentWeakPtr;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup = CustomSpline, ShowCategories = (Mobility), HideCategories = (Physics, Lighting, Mobile), meta = (BlueprintSpawnableComponent))
class CURVEBUILDER_API URuntimeCustomSplineBaseComponent : public URuntimeSplinePrimitiveComponent//, public IInterface_CollisionDataProvider
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSplineUpdated, URuntimeCustomSplineBaseComponent*, SplineComponent);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorAttached, AActor*, AttachedActor);
public:
	URuntimeCustomSplineBaseComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;

	virtual void OnComponentCreated() override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual void OnAttachmentChanged() override;

	virtual bool MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit = NULL, EMoveComponentFlags MoveFlags = MOVECOMP_NoFlags, ETeleportType Teleport = ETeleportType::None) override;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	//virtual FMatrix GetRenderMatrix() const override;

	//virtual UBodySetup* GetBodySetup() override { return BodySetup; }

	//virtual void OnCreatePhysicsState() override;

	//virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None) override;

	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	//virtual void CreateBodySetup() override;

	virtual void UpdateCollision() override;

	//virtual void SetDrawDebugCollision(bool bValue) override;

	virtual void InitializeCommandHelper() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditComponentMove(bool bFinished) override;
#endif

	virtual void Serialize(FArchive& Ar) override;

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void OnSplineUpdatedEvent();

	UFUNCTION(BlueprintNativeEvent, Category = "RuntimeCustomSpline|Update")
	void OnActorAttachedEvent(AActor* AttachedActor);

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|DrawInfo")
	void SetCustomSelected(bool bValue);

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	ERuntimeSplineType GetSplineType() const;

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Query")
	void GetParametricCubicPolynomialForms(
		TArray<FVector>& OutParamPoly3Forms,
		ECustomSplineCoordinateType TargetCoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Query")
	void GetCubicBezierForms(
		TArray<FVector>& OutBezierForms,
		ECustomSplineCoordinateType TargetCoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Query")
	void GetHermiteForms(
		TArray<FVector>& OutPositions, TArray<FVector>& OutArriveTangents, TArray<FVector>& OutLeaveTangents,
		ECustomSplineCoordinateType TargetCoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	//// Start interface of proxy.

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	ERuntimeSplineType GetType() const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	float GetLength(float Parameter) const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	float GetParameterAtLength(float Length) const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	int32 GetControlPointNum() const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	void GetSegmentParameters(TArray<float>& Parameters) const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	FVector GetPosition(float Parameter, ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal) const;
	
	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	FVector GetTangent(float Parameter, ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal) const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	float GetCurvature(float Parameter) const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	float GetPlanCurvature(float Parameter, int32 PlanIndex = 0) const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	FVector2D GetParameterRange() const;

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	bool FindParameterByPosition(float& Param, const FVector& Position, float ToleranceSqr = 1., ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal) const;
	
	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	bool FindParameterByComponentValue(TArray<float>& Params, float ComponentValue, int32 ComponentIndex, float ToleranceSqr = 1.) const;

	//// End interface of proxy.

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void Reverse();

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void ClearSpline();

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	URuntimeSplinePointBaseComponent* AddEndPoint(
		const FVector& Position,
		bool bAtLast = true, 
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	URuntimeSplinePointBaseComponent* InsertPoint(
		const FVector& Position,
		bool& bSucceedReturn,
		ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void UpdateControlPointsLocation();

public:
	virtual void ReverseImpl();

public:
	TWeakPtr<FSpatialSplineGraph3::FSplineType> GetSplineProxyWeakPtr() const;
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

	URuntimeSplinePointBaseComponent* AddPointInternal(const TSharedRef<FSpatialControlPoint3>& PointRef, int32 TangentFlag = 0);

	void UpdateTransformByCtrlPoint();

	static int32 SampleParameters(TArray<double>& OutParameters, const FSpatialSplineBase3& SplineInternal, double SegLength, bool bByCurveLength = false, bool bAdjustKeyLength = true);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Settings")
	TSubclassOf<URuntimeSplinePointBaseComponent> CustomSplinePointClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	TSet<URuntimeSplinePointBaseComponent*> PointComponents;

	UPROPERTY(AdvancedDisplay, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	URuntimeSplinePointBaseComponent* SelectedPoint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = "SetCustomSelected", Category = "RuntimeCustomSpline|Component")
	bool bCustomSelected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	bool bAutoSelectNewPoint = false;

	// Is parameter length or curve length? Currently use parameter length.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float DrawSegLength = 0.05f;//10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float DrawThickness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor CurveColor = FLinearColor(0.1f, 0.6f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor CtrlSegColor = FLinearColor(0.2f, 1.f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	bool bCreateCollisionByCurveLength = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	float CollisionSegLength = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	float CollisionSegWidth = 32.f;

	UPROPERTY(BlueprintAssignable, Category = "RuntimeCustomSpline|Update")
	FOnSplineUpdated OnSplineUpdateHandle;

	UPROPERTY(BlueprintAssignable, Category = "RuntimeCustomSpline|Update")
	FOnActorAttached OnActorAttachedHandle;

public:
	TSharedPtr<FSpatialSplineGraph3::FSplineWrapper> SplineBaseWrapperProxy;

private:
	bool bLastCreateCollisionByCurveLength = false;

	AActor* PreviousAttachedActor = nullptr;

};
