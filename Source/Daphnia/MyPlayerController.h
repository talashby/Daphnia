// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DAPHNIA_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
		AMyPlayerController();
		static AMyPlayerController* GetInstance();
		bool IsLeft() const;
		bool IsRight() const;
		bool IsUp() const;
		bool IsDown() const;
		bool IsForward() const;
		bool IsBackward() const;

protected:
	virtual void SetupInputComponent() override;
	bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;

	void TouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location);
	virtual void BeginPlay() override;
private:

	void ClickPressed(const FVector& Location);
	
	bool m_isLeft = false;
	bool m_isRight = false;
	bool m_isUp = false;
	bool m_isDown = false;
	bool m_isForward = false;
	bool m_isBackward = false;
};
