// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#pragma once

#include "OffsetExplicitBase.h"

template<int32 Degree>
template<int32 DimOri>
inline void TOffsetExplicit2Base<Degree>::MakeCurves(TArray<TBezierCurve<DimOri, Degree>>& OutCurves, const TSplineBase<DimOri, Degree>& InOriginalSpline) const
{
	TTuple<double, double> OriginalParamRange = InOriginalSpline.GetParamRange();
	static constexpr double InvDegDbl = 1. / static_cast<double>(Degree);
	TArray<TVectorX<DimOri> > OffDirs;
	TArray<TVectorX<DimOri> > OriV;
	TArray<TVectorX<DimOri> > OriT;
	TArray<double> OriP;
	TArray<TVectorX<2> > OffV;
	TArray<TVectorX<2> > OffT;

	TArray<double> KnotsS;
	GetKnotsS(KnotsS);

	OffsetDirs.Reserve(KnotsS.Num());
	OriV.Reserve(KnotsS.Num());
	OriT.Reserve(KnotsS.Num());
	OriP.Reserve(KnotsS.Num());
	OffV.Reserve(KnotsS.Num());
	OffT.Reserve(KnotsS.Num());
	OutCurves.Empty(KnotsS.Num() - 1);
	for (int32 i = 0; i < KnotsS.Num(); ++i)
	{
		TVectorX<DimOri>& CurOffDir = OffDirs.Add_GetRef(TVecLib<DimOri>::Zero());

		double CurKnotS = KnotsS[i];

		double& InCurP = OriP.Add_GetRef(InOriginalSpline.GetParameterAtLength(CurKnotS));
		if (InCurP > OriginalParamRange.Value)
		{
			InCurP = OriginalParamRange.Value;
		}

		TVectorX<DimOri>& CurV = OriV.Add_GetRef(OffsetInOriginalSpline.GetPosition(InCurP));
		TVectorX<DimOri>& CurT = OriT.Add_GetRef(InOriginalSpline.GetTangent(InCurP));

		TVectorX<2>& CurOffV = OffV.Add_GetRef(GetPosition(CurKnotS));
		TVectorX<2>& CurOffT = OffT.Add_GetRef(GetTangent(CurKnotS));

		//double CurTSize = TVecLib<DimOri>::Size(CurT);
		//CurOffT *= (CurTSize / CurOffT[0]);

		double CurTBiProjSize = CurT | CurOffDir;
		double CurTProjSize = sqrt(TVecLib<DimOri>::SizeSquared(CurT) - CurTBiProjSize * CurTBiProjSize);
		CurOffT *= CurTProjSize / CurOffT[0];

		switch (OffsetType.DirectionType)
		{
		case EOffsetDirectionType::DirZ:
			CurOffDir = FVector::UpVector * OffsetType.Sgn;
			break;
		case EOffsetDirectionType::DirT:
			CurOffDir = (FVector::UpVector ^ CurT).GetSafeNormal() * OffsetType.Sgn;
			break;
		}

		if (i > 0)
		{
			TVectorX<DimOri> PrevTargetV = OriV.Last(1) + OffDirs.Last(1) * TVecLib<2>::Last(OffV.Last(1));
			TVectorX<DimOri> CurTargetV = CurV + CurOffDir * TVecLib<2>::Last(CurOffV);
			TVectorX<DimOri> PrevTargetT = OriT.Last(1) + OffDirs.Last(1) * TVecLib<2>::Last(OffT.Last(1));
			TVectorX<DimOri> CurTargetT = CurT + CurOffDir * TVecLib<2>::Last(CurOffT);

			OutCurves.Add(TBezierCurve<DimOri, Degree>({
				PrevTargetV,
				PrevTargetV + PrevTargetT * InvDegDbl,
				CurTargetV - CurTargetT * InvDegDbl,
				CurTargetV, }));
		}
	}
}
