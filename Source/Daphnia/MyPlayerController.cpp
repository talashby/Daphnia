// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"
#include "ParallelPhysics/ObserverClient.h"
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
	if (PPh::ObserverClient::Instance() && (EventType == IE_Pressed || EventType == IE_Released))
	{
		bool state = EventType == IE_Pressed;
		if ("A" == Key.GetFName() || "Left" == Key.GetFName())
		{
			m_isLeft = state;
			PPh::ObserverClient::Instance()->SetIsLeft(state);
		}
		else if ("D" == Key.GetFName() || "Right" == Key.GetFName())
		{
			m_isRight = state;
			PPh::ObserverClient::Instance()->SetIsRight(state);
		}
		else if ("W" == Key.GetFName() || "Up" == Key.GetFName())
		{
			m_isUp = state;
			PPh::ObserverClient::Instance()->SetIsUp(state);
		}
		else if ("S" == Key.GetFName() || "Down" == Key.GetFName())
		{
			m_isDown = state;
			PPh::ObserverClient::Instance()->SetIsDown(state);
		}
		else if ("SpaceBar" == Key.GetFName())
		{
			m_isForward = state;
			PPh::ObserverClient::Instance()->SetIsForward(state);
		}
		else if ("Backslash" == Key.GetFName())
		{
			m_isBackward = state;
			PPh::ObserverClient::Instance()->SetIsBackward(state);
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

