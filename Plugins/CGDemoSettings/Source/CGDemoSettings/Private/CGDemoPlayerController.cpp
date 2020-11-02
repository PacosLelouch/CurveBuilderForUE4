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
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ACGDemoPlayerController::PressLeftMouseButton);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ACGDemoPlayerController::PressRightMouseButton);
	InputComponent->BindKey(EKeys::MiddleMouseButton, IE_Pressed, this, &ACGDemoPlayerController::PressMiddleMouseButton);

	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ACGDemoPlayerController::ReleaseLeftMouseButton);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &ACGDemoPlayerController::ReleaseRightMouseButton);
	InputComponent->BindKey(EKeys::MiddleMouseButton, IE_Released, this, &ACGDemoPlayerController::ReleaseMiddleMouseButton);

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

	InputComponent->BindKey(EKeys::Enter, IE_Released, this, &ACGDemoPlayerController::ReleaseEnter);
}

void ACGDemoPlayerController::BeginPlay()
{
	BindOnLeftMouseButtonPressed();
	BindOnRightMouseButtonPressed();
	BindOnMiddleMouseButtonPressed();

	BindOnLeftMouseButtonReleased();
	BindOnRightMouseButtonReleased();
	BindOnMiddleMouseButtonReleased();

	BindOnLeftCtrlPressed();
	BindOnRightCtrlPressed();
	BindOnLeftCtrlReleased();
	BindOnRightCtrlReleased();

	BindOnCtrlAndKey1Released();
	BindOnCtrlAndKey2Released();
	BindOnCtrlAndKey3Released();
	BindOnCtrlAndKey4Released();
	BindOnCtrlAndKey5Released();
	BindOnCtrlAndKey0Released();

	BindOnEnterReleased();
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

void ACGDemoPlayerController::PressLeftMouseButton()
{
	if (OnLeftMouseButtonPressed.IsBound()) {
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY)) {
			OnLeftMouseButtonPressed.Broadcast(EKeys::LeftMouseButton, FVector2D(MouseX, MouseY), IE_Pressed, this);
		}
	}
}

void ACGDemoPlayerController::PressRightMouseButton()
{
	if (OnRightMouseButtonPressed.IsBound()) {
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY)) {
			OnRightMouseButtonPressed.Broadcast(EKeys::RightMouseButton, FVector2D(MouseX, MouseY), IE_Pressed, this);
		}
	}
}

void ACGDemoPlayerController::PressMiddleMouseButton()
{
	if (OnMiddleMouseButtonPressed.IsBound()) {
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY)) {
			OnMiddleMouseButtonPressed.Broadcast(EKeys::MiddleMouseButton, FVector2D(MouseX, MouseY), IE_Pressed, this);
		}
	}
}

void ACGDemoPlayerController::ReleaseLeftMouseButton()
{
	if (OnLeftMouseButtonReleased.IsBound()) {
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY)) {
			OnLeftMouseButtonReleased.Broadcast(EKeys::LeftMouseButton, FVector2D(MouseX, MouseY), IE_Released, this);
		}
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

void ACGDemoPlayerController::ReleaseMiddleMouseButton()
{
	if (OnMiddleMouseButtonReleased.IsBound()) {
		float MouseX, MouseY;
		if (GetMousePosition(MouseX, MouseY)) {
			OnMiddleMouseButtonReleased.Broadcast(EKeys::MiddleMouseButton, FVector2D(MouseX, MouseY), IE_Released, this);
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

void ACGDemoPlayerController::ReleaseEnter()
{
	if (OnEnterReleased.IsBound()) {
		OnEnterReleased.Broadcast(EKeys::Enter, IE_Released, this);
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

void ACGDemoPlayerController::BindOnLeftMouseButtonPressed()
{
	OnLeftMouseButtonPressed.AddDynamic(this, &ACGDemoPlayerController::TestLeftMouseButtonPressed);
}

void ACGDemoPlayerController::BindOnRightMouseButtonPressed()
{
	OnRightMouseButtonPressed.AddDynamic(this, &ACGDemoPlayerController::TestRightMouseButtonPressed);
}

void ACGDemoPlayerController::BindOnMiddleMouseButtonPressed()
{
	OnMiddleMouseButtonPressed.AddDynamic(this, &ACGDemoPlayerController::TestMiddleMouseButtonPressed);
}

void ACGDemoPlayerController::BindOnLeftMouseButtonReleased()
{
	OnLeftMouseButtonReleased.AddDynamic(this, &ACGDemoPlayerController::TestLeftMouseButtonReleased);
}

void ACGDemoPlayerController::BindOnRightMouseButtonReleased()
{
	OnRightMouseButtonReleased.AddDynamic(this, &ACGDemoPlayerController::TestRightMouseButtonReleased);
}

void ACGDemoPlayerController::BindOnMiddleMouseButtonReleased()
{
	OnMiddleMouseButtonReleased.AddDynamic(this, &ACGDemoPlayerController::TestMiddleMouseButtonReleased);
}

void ACGDemoPlayerController::BindOnCtrlAndKey1Released()
{
	OnCtrlAndKey1Released.AddDynamic(this, &ACGDemoPlayerController::TestKey1Button);
}

void ACGDemoPlayerController::BindOnCtrlAndKey2Released()
{
}

void ACGDemoPlayerController::BindOnCtrlAndKey3Released()
{
}

void ACGDemoPlayerController::BindOnCtrlAndKey4Released()
{
}

void ACGDemoPlayerController::BindOnCtrlAndKey5Released()
{
}

void ACGDemoPlayerController::BindOnCtrlAndKey0Released()
{
}

void ACGDemoPlayerController::BindOnEnterReleased()
{
}

void ACGDemoPlayerController::TestLeftMouseButtonPressed(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Pressed: %s, %s"),
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString());
}

void ACGDemoPlayerController::TestRightMouseButtonPressed(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Right Mouse Button Pressed: %s, %s"),
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString());
}

void ACGDemoPlayerController::TestMiddleMouseButtonPressed(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Middle Mouse Button Pressed: %s, %s"),
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString());
}

void ACGDemoPlayerController::TestLeftMouseButtonReleased(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Left Mouse Button Released: %s, %s"),
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString());
}

void ACGDemoPlayerController::TestRightMouseButtonReleased(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Right Mouse Button Released: %s, %s"), 
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString());
}

void ACGDemoPlayerController::TestMiddleMouseButtonReleased(FKey Key, FVector2D MouseScreenPos, EInputEvent InputEvent, APlayerController* Ctrl)
{
	FVector WorldPos, WorldDir;
	Ctrl->DeprojectScreenPositionToWorld(MouseScreenPos.X, MouseScreenPos.Y, WorldPos, WorldDir);
	UE_LOG(LogTemp, Warning, TEXT("Middle Mouse Button Released: %s, %s"),
		*WorldPos.ToCompactString(), *WorldDir.ToCompactString());
}

void ACGDemoPlayerController::TestKey1Button(FKey Key, EInputEvent InputEvent, APlayerController* Ctrl)
{
	UE_LOG(LogTemp, Warning, TEXT("Key 1 Released: %s"),
		*Key.ToString());
}
