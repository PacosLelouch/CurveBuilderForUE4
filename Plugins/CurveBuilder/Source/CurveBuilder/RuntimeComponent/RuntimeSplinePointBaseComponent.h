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

class CURVEBUILDER_API FRuntimeSplinePointCommands : public TCommands<FRuntimeSplinePointCommands>
{
public:
	FRuntimeSplinePointCommands();

	virtual void RegisterCommands() override;

public:
	/** Split connection here */
	TSharedPtr<FUICommandInfo> SplitConnection;

	/** Connect and fill splines */
	TSharedPtr<FUICommandInfo> ConnectAndFillSplinesEnd;

	/** Connect and fill splines */
	TSharedPtr<FUICommandInfo> ConnectAndFillSplinesStart;

	/** Create and fill splines */
	TSharedPtr<FUICommandInfo> CreateAndConnectSplinesEnd;

	/** Create and fill splines */
	TSharedPtr<FUICommandInfo> CreateAndConnectSplinesStart;
};

class CURVEBUILDER_API FRuntimeSplinePointCommandHelper : public FRuntimeSplineCommandHelperBase
{
public:
	FRuntimeSplinePointCommandHelper(class URuntimeSplinePointBaseComponent* Component = nullptr)
		: FRuntimeSplineCommandHelperBase()
		, ComponentWeakPtr(Component)
	{
		FRuntimeSplinePointCommands::Register();
	}

	virtual void CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;

	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = false) override;

	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;

protected:
	bool IsPointAndSplineValid() const;
	bool IsGraphValid() const;
	bool CheckPointIsEndPointOrNot(EContactType& OutContactType) const;

	void OnSplitConnection();
	bool CanSplitConnection() const;

	void OnConnectAndFillSplines(bool bForward, bool bFillInSource);
	bool CanConnectAndFillSplines() const;

public:
	virtual void MapActions() override;

	virtual void GenerateContextMenuSections(FMenuBuilder& InMenuBuilder) const override;

public:
	TWeakObjectPtr<URuntimeSplinePointBaseComponent> ComponentWeakPtr;
};

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

	virtual void InitializeCommandHelper() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void PostEditComponentMove(bool bFinished) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|DrawInfo")
	void SetCustomSelected(bool bValue);

	// Deprecated function. Please use SetRelativeLocation to move point!
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void MoveTo_Deprecated(const FVector& Position, ECustomSplineCoordinateType CoordinateType = ECustomSplineCoordinateType::SplineGraphLocal);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|Update")
	void UpdateComponentLocationBySpline();

	UFUNCTION(BlueprintPure, Category = "RuntimeCustomSpline|Query")
	bool IsEndPoint(bool& bIsForwardEnd) const;

public:
	void MoveSplinePointInternal();

	bool IsEndPointOrNot(EContactType& OutContactType) const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	FVector SplineLocalPosition = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	int32 TangentFlag = 0;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	//URuntimeSplinePointBaseComponent* PrevPointForTangent = nullptr;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	//URuntimeSplinePointBaseComponent* NextPointForTangent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = "SetCustomSelected", Category = "RuntimeCustomSpline|Component")
	bool bCustomSelected = false;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	UPROPERTY(AdvancedDisplay, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	URuntimeCustomSplineBaseComponent* ParentSpline = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionInfo")
	float CollisionDiameter = 64.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	int32 DrawPointSize = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor CtrlPointColor = FLinearColor(0.4f, 0.8f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	FLinearColor SelectedCtrlPointColor = FLinearColor(0.9f, 0.3f, 0.1f);

public:
	TWeakPtr<FSpatialControlPoint3> SplinePointProxy;

private:
	bool bIsDuplicated = false;
};
