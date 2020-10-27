// Fill out your copyright notice in the Description page of Project Settings.


#include "CGDemoGameModeBase.h"
#include "CGDemoHUD.h"
#include "CGDemoPlayerController.h"

ACGDemoGameModeBase::ACGDemoGameModeBase(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	PlayerControllerClass = ACGDemoPlayerController::StaticClass();
	HUDClass = ACGDemoHUD::StaticClass();
}
