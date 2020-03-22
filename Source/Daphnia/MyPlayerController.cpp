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

bool AMyPlayerController::IsLeft() const
{
	return m_isLeft;
}

bool AMyPlayerController::IsRight() const
{
	return m_isRight;
}

bool AMyPlayerController::IsUp() const
{
	return m_isUp;
}

bool AMyPlayerController::IsDown() const
{
	return m_isDown;
}

bool AMyPlayerController::IsForward() const
{
	return m_isForward;
}

bool AMyPlayerController::IsBackward() const
{
	return m_isBackward;
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
	if (EventType == IE_Pressed)
	{
		if ("A" == Key.GetFName())
		{
			m_isLeft = true;
		}
		else if ("D" == Key.GetFName())
		{
			m_isRight = true;
		}
		else if ("W" == Key.GetFName())
		{
			m_isUp = true;
		}
		else if ("S" == Key.GetFName())
		{
			m_isDown = true;
		}
		else if ("SpaceBar" == Key.GetFName())
		{
			m_isForward = true;
		}
		else if ("LeftAlt" == Key.GetFName())
		{
			m_isBackward = true;
		}
	}
	else if (EventType == IE_Released)
	{
		if ("A" == Key.GetFName())
		{
			m_isLeft = false;
		}
		else if ("D" == Key.GetFName())
		{
			m_isRight = false;
		}
		else if ("W" == Key.GetFName())
		{
			m_isUp = false;
		}
		else if ("S" == Key.GetFName())
		{
			m_isDown = false;
		}
		else if ("SpaceBar" == Key.GetFName())
		{
			m_isForward = false;
		}
		else if ("LeftAlt" == Key.GetFName())
		{
			m_isBackward = false;
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

