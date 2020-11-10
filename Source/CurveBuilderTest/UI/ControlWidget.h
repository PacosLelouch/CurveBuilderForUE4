// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/SpinBox.h"
#include "Types/SlateEnums.h"
#include "../BezierStringTest/BezierStringTestPlayerController.h"
#include "ControlWidget.generated.h"

UCLASS(BlueprintType)
class UControlWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual bool Initialize() override;
	
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	virtual void BindValues();

	virtual void BindEvents();

public:
	UPROPERTY(BlueprintReadWrite, Category = "Controller")
	ABezierStringTestPlayerController* Controller;
};
