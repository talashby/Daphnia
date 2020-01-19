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

protected:
	virtual void SetupInputComponent() override;
	bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;

	void TouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location);
	virtual void BeginPlay() override;
private:

	void ClickPressed(const FVector& Location);
};
