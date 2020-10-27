// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/


#include "CurveBuilderTestGameModeBase.h"
#include "CurveBuilderTestPlayerController.h"

ACurveBuilderTestGameModeBase::ACurveBuilderTestGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ACurveBuilderTestPlayerController::StaticClass();
}