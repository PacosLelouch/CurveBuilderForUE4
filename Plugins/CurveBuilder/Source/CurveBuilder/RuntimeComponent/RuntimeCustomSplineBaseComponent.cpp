// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "RuntimeCustomSplineBaseComponent.h"
#include "PrimitiveSceneProxy.h"

static constexpr bool bDrawLineByCurveLength = false;

template<int32 Dim>
struct TRuntimeSplineDrawInfo<Dim, 3>
{
	TRuntimeSplineDrawInfo(const URuntimeCustomSplineBaseComponent* InComponent)
		: LineColor(InComponent->LineColor)
		, SegLength(InComponent->SegLength)
		, Thickness(InComponent->Thickness)
		, DepthBias(InComponent->DepthBias)
		, SplineInternalRef(*InComponent->GetSplineProxy())
	{}
	FLinearColor LineColor = FLinearColor::White;
	float SegLength = 5.f;
	float Thickness = 0.f;
	float DepthBias = 0.f;
	const TSplineBase<Dim, 3>& SplineInternalRef;
};

class FRuntimeCustomSplineSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FRuntimeCustomSplineSceneProxy(const URuntimeCustomSplineBaseComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, SplineComponent(InComponent)
	{}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_RuntimeCustomSplineSceneProxy_GetDynamicMeshElements);

		if ((!IsValid(SplineComponent)) || (!SplineComponent->bSelected))
		{
			return;
		}

		auto* SplineProxy = SplineComponent->GetSplineProxy();
		if (!SplineProxy)
		{
			return;
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

				const FMatrix& LocalToWorldMat = GetLocalToWorld();

				// Taking into account the min and maximum drawing distance
				const float DistanceSqr = (View->ViewMatrices.GetViewOrigin() - LocalToWorldMat.GetOrigin()).SizeSquared();
				if (DistanceSqr < FMath::Square(GetMinDrawDistance()) || DistanceSqr > FMath::Square(GetMaxDrawDistance()))
				{
					continue;
				}

				FSpatial3DrawInfo DrawInfo(SplineComponent);
				
				URuntimeCustomSplineBaseComponent::DrawRuntimeSpline(PDI, View, DrawInfo, LocalToWorldMat, ESceneDepthPriorityGroup::SDPG_Foreground);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsValid(SplineComponent) && IsShown(View);//bDrawDebug && !IsSelected() && IsShown(View) && View->Family->EngineShowFlags.Splines;
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return sizeof *this + GetAllocatedSize(); }
	uint32 GetAllocatedSize(void) const { return FPrimitiveSceneProxy::GetAllocatedSize(); }

protected:
	const URuntimeCustomSplineBaseComponent* SplineComponent;
};

void URuntimeCustomSplineBaseComponent::BeginPlay()
{
	Super::BeginPlay(); 
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

FPrimitiveSceneProxy* URuntimeCustomSplineBaseComponent::CreateSceneProxy()
{
	return new FRuntimeCustomSplineSceneProxy(this);
}

FSpatialSplineGraph3::FSplineType* URuntimeCustomSplineBaseComponent::GetSplineProxy() const
{
	if (SplineBaseWrapperProxy && SplineBaseWrapperProxy->Spline)
	{
		switch (SplineBaseWrapperProxy->Spline->GetType())
		{
		case ESplineType::ClampedBSpline:
			return GetSplineProxyInternal<TSplineTraitByType<ESplineType::ClampedBSpline>::FSplineType>();
		case ESplineType::BezierString:
			return GetSplineProxyInternal<TSplineTraitByType<ESplineType::BezierString>::FSplineType>();
		}
	}
	return GetSplineProxyInternal<TSplineTraitByType<ESplineType::Unknown>::FSplineType>();
}

//template<>
void URuntimeCustomSplineBaseComponent::DrawRuntimeSpline(FPrimitiveDrawInterface* PDI, const FSceneView* View, const FSpatial3DrawInfo& DrawInfo, const FMatrix& LocalToWorld, uint8 DepthPriorityGroup)
{
	TTuple<double, double> ParamRange = DrawInfo.SplineInternalRef.GetParamRange();
	if (bDrawLineByCurveLength)
	{
		double Length = DrawInfo.SplineInternalRef.GetLength(ParamRange.Get<1>());

		double SegNumDbl = FMath::CeilToDouble(Length / static_cast<double>(DrawInfo.SegLength));//FMath::CeilToDouble(Length * static_cast<double>(DrawInfo.NumSteps));
		int32 SegNum = FMath::RoundToInt(SegNumDbl);
		double StepLength = Length / SegNumDbl;

		double NextS = 0.;
		double T = ParamRange.Get<0>();
		FVector Start = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
		for (int32 i = 0; i < SegNum; ++i)
		{
			NextS += StepLength;
			T = DrawInfo.SplineInternalRef.GetParameterAtLength(NextS);

			FVector End = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
			PDI->DrawLine(Start, End, DrawInfo.LineColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
			Start = End;
		}
	}
	else
	{
		double DiffParam = ParamRange.Get<1>() - ParamRange.Get<0>();
		double SegNumDbl = FMath::CeilToDouble(DiffParam / static_cast<double>(DrawInfo.SegLength));//FMath::CeilToDouble(Length * static_cast<double>(DrawInfo.NumSteps));
		int32 SegNum = FMath::RoundToInt(SegNumDbl);
		double StepParam = DiffParam / SegNumDbl;

		double T = ParamRange.Get<0>();
		FVector Start = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
		for (int32 i = 0; i < SegNum; ++i)
		{
			T += StepParam;

			FVector End = LocalToWorld.TransformPosition(DrawInfo.SplineInternalRef.GetPosition(T));
			PDI->DrawLine(Start, End, DrawInfo.LineColor, DepthPriorityGroup, DrawInfo.Thickness, DrawInfo.DepthBias, false);
			Start = End;
		}
	}

}
