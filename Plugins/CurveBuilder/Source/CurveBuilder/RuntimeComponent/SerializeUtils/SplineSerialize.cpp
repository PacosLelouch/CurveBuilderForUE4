// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "SplineSerialize.h"
#include "../../Compute/Splines/SplineBase.h"
#include "../RuntimeSplineGraph.h"
#include "../RuntimeCustomSplineBaseComponent.h"
#include "../RuntimeSplinePointBaseComponent.h"

bool SplineSerializeUtils::ShouldLoadArchive(FArchive& Ar)
{
	return 
		//Ar.IsByteSwapping() ||
		//Ar.IsCooking() ||
		//Ar.IsCountingMemory() ||
		//Ar.IsObjectReferenceCollector() ||
		//Ar.IsTransacting() ||
		Ar.IsLoading();
}

bool SplineSerializeUtils::ShouldSaveArchive(FArchive& Ar)
{
	return
		//Ar.IsByteSwapping() ||
		//Ar.IsCooking() ||
		Ar.IsCountingMemory() ||
		Ar.IsObjectReferenceCollector() ||
		//Ar.IsTransacting() ||
		Ar.IsSaving();
}

void SplineSerializeUtils::SerializeSplineWrapper(FArchive& Ar, FSpatialSplineGraph3::FSplineWrapper* SplineWrapper, uint8& bValidSerialize)
{
	ESplineType SplineType = ESplineType::Unknown;
	uint32 Major = SplineDataVersion::Major, Minor = SplineDataVersion::Minor;
	if (SplineSerializeUtils::ShouldSaveArchive(Ar))
	{
		if (!SplineWrapper || !SplineWrapper->Spline.IsValid())
		{
			bValidSerialize = false;
		}
		Ar << bValidSerialize;
		if (!bValidSerialize)
		{
			return;
		}
		Ar << Major;
		Ar << Minor;
		auto* Spline = SplineWrapper->Spline.Get();
		SplineType = Spline->GetType();
		Ar << SplineType;
	}
	else //if (SplineSerializeUtils::ShouldLoadArchive(Ar))
	{
		Ar << bValidSerialize;
		if (!bValidSerialize)
		{
			return;
		}
		Ar << Major;
		Ar << Minor;
		if (Major != SplineDataVersion::Major || Minor != SplineDataVersion::Minor)
		{
			return;
		}
		Ar << SplineType;
		switch (SplineType)
		{
		case ESplineType::ClampedBSpline:
			SplineWrapper->Spline = MakeShareable(new TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType());
			break;
		case ESplineType::BezierString:
			SplineWrapper->Spline = MakeShareable(new TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType());
			break;
		default:
			return;
		}
	}
	SerializeSpline(Ar, SplineWrapper->Spline.Get(), SplineType);
}

void SplineSerializeUtils::SerializeSpline(FArchive& Ar, FSpatialSplineGraph3::FSplineType* Spline, ESplineType SpType)
{
	switch (SpType)
	{
	case ESplineType::ClampedBSpline:
	{
		auto* BSpline = static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FSplineType*>(Spline);
		TArray<FVector4> ControlPointPos;
		TArray<double> KnotIntervals;
		if (Ar.IsSaving() || Ar.IsObjectReferenceCollector() || Ar.IsCountingMemory())
		{
			BSpline->GetCtrlPoints(ControlPointPos);
			BSpline->GetKnotIntervals(KnotIntervals);
			Ar << ControlPointPos;
			Ar << KnotIntervals;
		}
		else //if (SplineSerializeUtils::ShouldLoadArchive(Ar))
		{
			Ar << ControlPointPos;
			Ar << KnotIntervals;
			BSpline->Reset(ControlPointPos, KnotIntervals);
		}
	}
		break;
	case ESplineType::BezierString:
	{
		auto* Beziers = static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FSplineType*>(Spline);
		TArray<FVector4> ControlPointPos;
		TArray<FVector4> ControlPointPrev;
		TArray<FVector4> ControlPointNext;
		TArray<double> KnotIntervals;
		TArray<EEndPointContinuity> Continuities;
		if (Ar.IsSaving() || Ar.IsObjectReferenceCollector() || Ar.IsCountingMemory())
		{
			TArray<TWeakPtr<FSpatialControlPoint3> > CtrlPointStructs;
			Beziers->GetCtrlPointStructs(CtrlPointStructs);
			ControlPointPos.Reserve(CtrlPointStructs.Num());
			ControlPointPrev.Reserve(CtrlPointStructs.Num());
			ControlPointNext.Reserve(CtrlPointStructs.Num());
			KnotIntervals.Reserve(CtrlPointStructs.Num());
			Continuities.Reserve(CtrlPointStructs.Num());
			for (int32 i = 0; i < CtrlPointStructs.Num(); ++i)
			{
				auto* CtrlPointPtr = 
					static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FControlPointType*>(CtrlPointStructs[i].Pin().Get());
				ControlPointPos.Add(CtrlPointPtr->Pos);
				ControlPointPrev.Add(CtrlPointPtr->PrevCtrlPointPos);
				ControlPointNext.Add(CtrlPointPtr->NextCtrlPointPos);
				KnotIntervals.Add(CtrlPointPtr->Param);
				Continuities.Add(CtrlPointPtr->Continuity);
			}
			Ar << ControlPointPos;
			Ar << ControlPointPrev;
			Ar << ControlPointNext;
			Ar << KnotIntervals;
			Ar << Continuities;
		}
		else //if (Ar.IsLoading())
		{
			Ar << ControlPointPos;
			Ar << ControlPointPrev;
			Ar << ControlPointNext;
			Ar << KnotIntervals;
			Ar << Continuities;
			Beziers->Reset(ControlPointPos, ControlPointPrev, ControlPointNext, KnotIntervals, Continuities);
		}
	}
		break;
	default:
		return;
	}
}

void SplineSerializeUtils::DistributeSplinePoints(TSet<URuntimeSplinePointBaseComponent*>& PointComponents, FSpatialSplineGraph3::FSplineType* Spline)
{
	int32 StructIdx = 0;
	int32 CompIdx = 0;
	TArray<TWeakPtr<FSpatialControlPoint3> > PointStructs;
	Spline->GetCtrlPointStructs(PointStructs);
	for (URuntimeSplinePointBaseComponent* PointComp : PointComponents)
	{
		if (StructIdx >= PointStructs.Num())
		{
			break;
		}
		switch (Spline->GetType())
		{
		case ESplineType::ClampedBSpline:
		{
			PointComp->SplinePointProxy = PointStructs[StructIdx];
			PointComp->TangentFlag = 0;
			if (PointStructs[StructIdx].IsValid())
			{
				const FSpatialControlPoint3& PointStruct =
					*static_cast<TSplineTraitByType<ESplineType::ClampedBSpline, 3, 3>::FControlPointType*>(PointStructs[StructIdx].Pin().Get());
				PointComp->SplineLocalPosition = TVecLib<4>::Projection(PointStruct.Pos);
			}
			++StructIdx;
		}
			break;
		case ESplineType::BezierString:
		{
			static int32 TangentFlagMap[3] = { 0, -1, 1 };
			int32 Mod = CompIdx % 3;
			PointComp->SplinePointProxy = PointStructs[StructIdx];
			PointComp->TangentFlag = TangentFlagMap[Mod];
			if (PointStructs[StructIdx].IsValid())
			{
				const auto& PointStruct = 
					*static_cast<TSplineTraitByType<ESplineType::BezierString, 3, 3>::FControlPointType*>(PointStructs[StructIdx].Pin().Get());
				switch (PointComp->TangentFlag)
				{
				case 0:
					PointComp->SplineLocalPosition = TVecLib<4>::Projection(PointStruct.Pos);
					break;
				case -1:
					PointComp->SplineLocalPosition = TVecLib<4>::Projection(PointStruct.PrevCtrlPointPos);
					break;
				case 1:
					PointComp->SplineLocalPosition = TVecLib<4>::Projection(PointStruct.NextCtrlPointPos);
					break;
				}
			}
			if (Mod == 2)
			{
				++StructIdx;
			}
		}
			break;
		}
		++CompIdx;
	}
}
