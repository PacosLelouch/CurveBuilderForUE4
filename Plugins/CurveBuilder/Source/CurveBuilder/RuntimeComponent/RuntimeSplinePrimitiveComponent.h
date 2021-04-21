// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "CoreMinimal.h"
#include "RuntimeSplineGraph.h"
#include "UObject/ObjectMacros.h"
#include "Components/PrimitiveComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/InputChord.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandList.h"
#include "RuntimeSplinePrimitiveComponent.generated.h"

class FUICommandInfo;
class ISlateStyle;
class SWidget;
class FMenuBuilder;

#ifdef CONCATENATE_FOR_PREPROCESSOR
#undef CONCATENATE_FOR_PREPROCESSOR
#endif
#define CONCATENATE_FOR_PREPROCESSOR(A,B) A ## B
#ifdef CONCATENATE_FOR_PREPROCESSOR_WRAPPER
#undef CONCATENATE_FOR_PREPROCESSOR_WRAPPER
#endif
#define CONCATENATE_FOR_PREPROCESSOR_WRAPPER(A,B) CONCATENATE_FOR_PREPROCESSOR(A, B)

#define ENABLE_THREAD_MUTEX_LOCK 1

#if ENABLE_THREAD_MUTEX_LOCK
// Execute with mutex lock.
// Use a unique pointer to lock the mutex, in case of the code includes return.
#define EXEC_WITH_THREAD_MUTEX_LOCK(MuteX,...) \
		FScopeLockManual CONCATENATE_FOR_PREPROCESSOR_WRAPPER(Lock, __LINE__)(&(MuteX)); \
		__VA_ARGS__ \
		CONCATENATE_FOR_PREPROCESSOR_WRAPPER(Lock, __LINE__).UnlockManually()

	//#define EXEC_WITH_THREAD_MUTEX_LOCK(MuteX,...) \
	//	TUniquePtr<FScopeLock> CONCATENATE_FOR_PREPROCESSOR_WRAPPER(Lock, __LINE__) = MakeUnique<FScopeLock>(&(MuteX)); \
	//	__VA_ARGS__ \
	//	CONCATENATE_FOR_PREPROCESSOR_WRAPPER(Lock, __LINE__).Reset()

#define SCOPE_MUTEX_LOCK(MuteX) FScopeLock CONCATENATE_FOR_PREPROCESSOR_WRAPPER(Lock, __LINE__)(&(MuteX))
#else
// Just execute without mutex.
#define EXEC_WITH_THREAD_MUTEX_LOCK(MuteX,...) __VA_ARGS__

#define SCOPE_MUTEX_LOCK(MuteX) 
#endif

class CURVEBUILDER_API FScopeLockManual
{
public:
	FScopeLockManual(FCriticalSection* InSynchObject)
		: SynchObject(InSynchObject)
	{
		if (SynchObject)
		{
			SynchObject->Lock();
		}
	}

	void UnlockManually()
	{
		if (SynchObject)
		{
			SynchObject->Unlock();
			SynchObject = nullptr;
		}
	}

	/** Destructor that performs a release on the synchronization object. */
	~FScopeLockManual()
	{
		if (SynchObject)
		{
			SynchObject->Unlock();
		}
	}
private:

	/** Default constructor (hidden on purpose). */
	FScopeLockManual() = delete;

	/** Copy constructor( hidden on purpose). */
	FScopeLockManual(const FScopeLock& InScopeLock) = delete;

	/** Assignment operator (hidden on purpose). */
	FScopeLockManual& operator=(FScopeLock& InScopeLock) = delete;

private:

	// Holds the synchronization object to aggregate and scope manage.
	FCriticalSection* SynchObject;
};

class CURVEBUILDER_API FRuntimeSplineCommandsBase : public TCommands<FRuntimeSplineCommandsBase>
{
public:
	FRuntimeSplineCommandsBase();

	virtual void RegisterCommands() override;

public:
	///** Delete key */
	//TSharedPtr<FUICommandInfo> DeleteKey;

	///** Duplicate key */
	//TSharedPtr<FUICommandInfo> DuplicateKey;

	///** Add key */
	//TSharedPtr<FUICommandInfo> AddKey;

	///** Select all */
	//TSharedPtr<FUICommandInfo> SelectAll;

	///** Reset to unclamped tangent */
	//TSharedPtr<FUICommandInfo> ResetToUnclampedTangent;

	///** Reset to clamped tangent */
	//TSharedPtr<FUICommandInfo> ResetToClampedTangent;

	///** Set spline key to Curve type */
	//TSharedPtr<FUICommandInfo> SetKeyToCurve;

	///** Set spline key to Linear type */
	//TSharedPtr<FUICommandInfo> SetKeyToLinear;

	///** Set spline key to Constant type */
	//TSharedPtr<FUICommandInfo> SetKeyToConstant;

	///** Focus on selection */
	//TSharedPtr<FUICommandInfo> FocusViewportToSelection;

	///** Snap to nearest spline point on another spline component */
	//TSharedPtr<FUICommandInfo> SnapToNearestSplinePoint;

	///** Align to nearest spline point on another spline component */
	//TSharedPtr<FUICommandInfo> AlignToNearestSplinePoint;

	///** Align perpendicular to nearest spline point on another spline component */
	//TSharedPtr<FUICommandInfo> AlignPerpendicularToNearestSplinePoint;

	///** Snap all spline points to selected point X */
	//TSharedPtr<FUICommandInfo> SnapAllToSelectedX;

	///** Snap all spline points to selected point Y */
	//TSharedPtr<FUICommandInfo> SnapAllToSelectedY;

	///** Snap all spline points to selected point Z */
	//TSharedPtr<FUICommandInfo> SnapAllToSelectedZ;

	///** No axis is locked when adding new spline points */
	//TSharedPtr<FUICommandInfo> SetLockedAxisNone;

	///** Lock X axis when adding new spline points */
	//TSharedPtr<FUICommandInfo> SetLockedAxisX;

	///** Lock Y axis when adding new spline points */
	//TSharedPtr<FUICommandInfo> SetLockedAxisY;

	///** Lock Z axis when adding new spline points */
	//TSharedPtr<FUICommandInfo> SetLockedAxisZ;

	///** Whether the visualization should show roll and scale */
	//TSharedPtr<FUICommandInfo> VisualizeRollAndScale;

	///** Whether we allow separate Arrive / Leave tangents, resulting in a discontinuous spline */
	//TSharedPtr<FUICommandInfo> DiscontinuousSpline;

	///** Reset this spline to its default */
	//TSharedPtr<FUICommandInfo> ResetToDefault;
};

class CURVEBUILDER_API FRuntimeSplineCommandHelperBase : public TSharedFromThis<FRuntimeSplineCommandHelperBase>
{
public:
	FRuntimeSplineCommandHelperBase()
		: CommandList(MakeShareable(new FUICommandList()))
	{
	}

	virtual ~FRuntimeSplineCommandHelperBase() {}

	virtual void MapActions();

	virtual TSharedPtr<SWidget> GenerateContextMenu();

	virtual void GenerateContextMenuSections(FMenuBuilder& InMenuBuilder) const;

	virtual void CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY);

	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = false);

	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false);

	virtual bool CreateMenuBarAt(const FVector& SnappedWorldPosition = FVector::ZeroVector, TSharedPtr<IMenu>* OpenedMenuPtr = nullptr);

public:
	TSharedPtr<FUICommandList> CommandList;

	TOptional<FVector> LastSnappedWorldPosition;

public:
	static ISlateStyle& GetSlateStyle();

	static TSharedPtr<ISlateStyle> SlateStyle;
};

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

	virtual void SetDrawInGame(bool bValue);

	virtual void InitializeCommandHelper();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	FTransform GetSplineLocalToWorldTransform() const;
	FTransform GetWorldToSplineLocalTransform() const;
	FTransform GetSplineLocalToComponentLocalTransform() const;
	FTransform GetComponentLocalToSplineLocalTransform() const;
	FTransform GetWorldToParentComponentTransform() const;
	FTransform GetParentComponentToWorldTransform() const;
	FTransform GetSplineLocalToParentComponentTransform() const;
	FTransform GetParentComponentToSplineLocalTransform() const;

	FVector ConvertPosition(const FVector& SourcePosition, ECustomSplineCoordinateType From, ECustomSplineCoordinateType To) const;

	virtual void CreateBodySetup();

	virtual void UpdateCollision();

public:
	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|UI")
	void OpenMenu(const FVector& SnappedWorldPosition);

	UFUNCTION(BlueprintCallable, Category = "RuntimeCustomSpline|UI")
	void CloseMenu();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|Component")
	ARuntimeSplineGraph* ParentGraph = nullptr;

	UPROPERTY(Instanced)
	UBodySetup* BodySetup = nullptr;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	bool bDrawInGame = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|DrawInfo")
	float DepthBias = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionDebugInfo")
	float DebugCollisionLineWidth = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionDebugInfo")
	bool bDrawDebugCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RuntimeCustomSpline|CollisionDebugInfo")
	FLinearColor DebugCollisionColor = FLinearColor(0.2f, 0.7f, 0.2f);

public:
	TSharedPtr<FRuntimeSplineCommandHelperBase> CommandHelper;

	TSharedPtr<IMenu> OpenedMenu;

	mutable FCriticalSection RenderMuteX;

public:
	static ECollisionTraceFlag SpCompCollisionTraceFlag;
};
