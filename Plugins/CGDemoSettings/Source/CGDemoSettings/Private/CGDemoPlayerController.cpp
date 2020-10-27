// Fill out your copyright notice in the Description page of Project Settings.


#include "CGDemoPlayerController.h"
#include "Components/InputComponent.h"


ACGDemoPlayerController::ACGDemoPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	ClickEventKeys = { EKeys::LeftMouseButton, EKeys::MiddleMouseButton, EKeys::RightMouseButton };
}

void ACGDemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ACGDemoPlayerController::ReleaseRightMouseButton);

	InputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this, &ACGDemoPlayerController::PressLeftCtrl);
	InputComponent->BindKey(EKeys::RightControl, IE_Pressed, this, &ACGDemoPlayerController::PressRightCtrl);
	InputComponent->BindKey(EKeys::LeftControl, IE_Released, this, &ACGDemoPlayerController::ReleaseLeftCtrl);
	InputComponent->BindKey(EKeys::RightControl, IE_Released, this, &ACGDemoPlayerController::ReleaseRightCtrl);

	InputComponent->BindKey(EKeys::One, IE_Released, this, &ACGDemoPlayerController::ReleaseKey1);
	InputComponent->BindKey(EKeys::Two, IE_Released, this, &ACGDemoPlayerController::ReleaseKey2);
	InputComponent->BindKey(EKeys::Three, IE_Released, this, &ACGDemoPlayerController::ReleaseKey3);
	InputComponent->BindKey(EKeys::Four, IE_Released, this, &ACGDemoPlayerController::ReleaseKey4);
	InputComponent->BindKey(EKeys::Five, IE_Released, this, &ACGDemoPlayerController::ReleaseKey5);
	InputComponent->BindKey(EKeys::Zero, IE_Released, this, &ACGDemoPlayerController::ReleaseKey0);
}

void ACGDemoPlayerController::BeginPlay()
{
	BindOnRightMouseButtonReleased();

	BindOnLeftCtrlPressed();
	BindOnRightCtrlPressed();
	BindOnLeftCtrlReleased();
	BindOnRightCtrlReleased();

	BindOnKey1Released();
	BindOnKey2Released();
	BindOnKey3Released();
	BindOnKey4Released();
	BindOnKey5Released();
	BindOnKey0Released();
}

void ACGDemoPlayerController::PressLeftCtrl()
{
	bPressedCtrl = true;
	if (OnLeftCtrlPressed.IsBound()) {
		OnLeftCtrlPressed.Broadcast(EKeys::RightControl, IE_Pressed, this);
	}
}

void ACGDemoPlayerController::PressRightCtrl()
{
	bPressedCtrl = true;
	if (OnRightCtrlPressed.IsBound()) {
		OnRightCtrlPressed.Broadcast(EKeys::RightControl, IE_Pressed, this);
	}
}

void ACGDemoPlayerController::ReleaseLeftCtrl()
{
	bPressedCtrl = false;
	if (OnLeftCtrlReleased.IsBound()) {
		OnLeftCtrlReleased.Broadcast(EKeys::RightControl, IE_Released, this);
	}
}

void ACGDemoPlayerController::ReleaseRightCtrl()
{
	bPressedCtrl = false;
	if (OnRightCtrlReleased.IsBound()) {
		OnRightCtrlReleased.Broadcast(EKeys::RightControl, IE_Released, this);
	}
}

void ACGDemoPlayerController::ReleaseRightMouseButton()
{
	if (OnRightMouseButtonReleased.IsBound()) {
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY)) {
			OnRightMouseButtonReleased.Broadcast(EKeys::RightMouseButton, FVector2D(MouseX, MouseY), IE_Released, this);
		}
	}
}

void ACGDemoPlayerController::ReleaseKey1()
{
	if (bPressedCtrl && OnCtrlAndKey1Released.IsBound()) {
		OnCtrlAndKey1Released.Broadcast(EKeys::One, IE_Released, this);
	}
}

void ACGDemoPlayerController::ReleaseKey2()
{
	if (bPressedCtrl && OnCtrlAndKey2Released.IsBound()) {
		OnCtrlAndKey2Released.Broadcast(EKeys::Two, IE_Released, this);
	}
}

void ACGDemoPlayerController::ReleaseKey3()
{
	if (bPressedCtrl && OnCtrlAndKey3Released.IsBound()) {
		OnCtrlAndKey3Released.Broadcast(EKeys::Three, IE_Released, this);
	}
}

void ACGDemoPlayerController::ReleaseKey4()
{
	if (bPressedCtrl && OnCtrlAndKey4Released.IsBound()) {
		OnCtrlAndKey4Released.Broadcast(EKeys::Four, IE_Released, this);
	}
}

void ACGDemoPlayerController::ReleaseKey5()
{
	if (bPressedCtrl && OnCtrlAndKey5Released.IsBound()) {
		OnCtrlAndKey5Released.Broadcast(EKeys::Five, IE_Released, this);
	}
}

void ACGDemoPlayerController::ReleaseKey0()
{
	if (bPressedCtrl && OnCtrlAndKey0Released.IsBound()) {
		OnCtrlAndKey0Released.Broadcast(EKeys::Zero, IE_Released, this);
	}
}

void ACGDemoPlayerController::BindOnLeftCtrlPressed()
{
}

void ACGDemoPlayerController::BindOnRightCtrlPressed()
{
}

void ACGDemoPlayerController::BindOnLeftCtrlReleased()
{
}

void ACGDemoPlayerController::BindOnRightCtrlReleased()
{
}

void ACGDemoPlayerController::BindOnRightMouseButtonReleased()
{
	OnRightMouseButtonReleased.AddDynamic(this, &ACGDemoPlayerController::TestRightMouseButton);
}

void ACGDemoPlayerController::BindOnKey1Released()
{
	OnCtrlAndKey1Released.AddDynamic(this, &ACGDemoPlayerController::TestKey1Button);
}

void ACGDemoPlayerController::BindOnKey2Released()
{
}

void ACGDemoPlayerController::BindOnKey3Released()
{
}

void ACGDemoPlayerController::BindOnKey4Released()
{
}

void ACGDemoPlayerController::BindOnKey5Released()
{
}

void ACGDemoPlayerController::BindOnKey0Released()
{
}

void ACGDemoPlayerController::TestRightMouseButton(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Right Mouse Button Released: %s, %s"), 
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString());
}

void ACGDemoPlayerController::TestKey1Button(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Key 1 Released: %s"),
		*Key.ToString());
}
