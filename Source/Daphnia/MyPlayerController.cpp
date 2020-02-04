// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

#include "DaphniaPawn.h"

static AMyPlayerController* s_InstancePtr;

AMyPlayerController::AMyPlayerController() : APlayerController()
{
	bEnableClickEvents = true;
	bEnableTouchEvents = true;
	bShowMouseCursor = true;
}

AMyPlayerController* AMyPlayerController::GetInstance()
{
	check(s_InstancePtr);
	return s_InstancePtr;
}

void AMyPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMyPlayerController::TouchPressed);

	SetInputMode(FInputModeGameOnly()); // to focus to game when starts in editor
	SetInputMode(FInputModeGameAndUI()); // remove mouse capture
}

bool AMyPlayerController::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	float fLocationX, fLocationY;
	GetMousePosition(fLocationX, fLocationY);
	if ("LeftMouseButton" == Key.GetFName())
	{
		if (EInputEvent::IE_Pressed == EventType || EInputEvent::IE_DoubleClick == EventType)
		{
			ClickPressed(FVector(fLocationX, fLocationY, 0));
		}
	}

	return Super::InputKey(Key, EventType, AmountDepressed, bGamepad);
}

void AMyPlayerController::TouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	ClickPressed(Location);
}

void AMyPlayerController::BeginPlay()
{
	s_InstancePtr = this;
}

void AMyPlayerController::ClickPressed(const FVector& Location)
{
}

