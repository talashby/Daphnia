// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyHudWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAPHNIA_API UMyHudWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	UFUNCTION(BlueprintCallable, Category = "BlueprintCallable")
	void SwitchCameraView();
};
