// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CGDemoPlayerController.generated.h"

class UInputComponent;

/**
 * 
 */
UCLASS()
class CGDEMOSETTINGS_API ACGDemoPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnMouseEventHandler,
		FKey, Key,
		FVector2D, MouseScreenPos,
		EInputEvent, InputEvent,
		APlayerController*, Controller);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnKeyEventHandler,
		FKey, Key,
		EInputEvent, InputEvent,
		APlayerController*, Controller);

	ACGDemoPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void SetupInputComponent() override;

	virtual void BeginPlay() override;

public:
	void PressLeftCtrl();
	void PressRightCtrl();
	void ReleaseLeftCtrl();
	void ReleaseRightCtrl();
	void ReleaseRightMouseButton();
	void ReleaseKey1();
	void ReleaseKey2();
	void ReleaseKey3();
	void ReleaseKey4();
	void ReleaseKey5();
	void ReleaseKey0();
	void ReleaseEnter();

	virtual void BindOnLeftCtrlPressed();
	virtual void BindOnRightCtrlPressed();
	virtual void BindOnLeftCtrlReleased();
	virtual void BindOnRightCtrlReleased();
	virtual void BindOnRightMouseButtonReleased();
	virtual void BindOnCtrlAndKey1Released();
	virtual void BindOnCtrlAndKey2Released();
	virtual void BindOnCtrlAndKey3Released();
	virtual void BindOnCtrlAndKey4Released();
	virtual void BindOnCtrlAndKey5Released();
	virtual void BindOnCtrlAndKey0Released();
	virtual void BindOnEnterReleased();
public:
	bool bPressedCtrl = false;

	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnLeftCtrlPressed;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnRightCtrlPressed;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnLeftCtrlReleased;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnRightCtrlReleased;

	UPROPERTY(BlueprintAssignable)
	FOnMouseEventHandler OnRightMouseButtonReleased;

	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnCtrlAndKey1Released;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnCtrlAndKey2Released;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnCtrlAndKey3Released;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnCtrlAndKey4Released;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnCtrlAndKey5Released;
	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnCtrlAndKey0Released;

	UPROPERTY(BlueprintAssignable)
	FOnKeyEventHandler OnEnterReleased;

private:
	UFUNCTION()
	void TestRightMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl);

	UFUNCTION()
	void TestKey1Button(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl);
};
