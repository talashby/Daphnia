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

public:
	UMyHudWidget();

	static UMyHudWidget* GetInstance();
protected:
	void NativeOnInitialized() override;
	void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;
	// Button_SwitchCamera
	UFUNCTION(BlueprintCallable, Category = "BlueprintCallable")
	void SwitchCameraView();

	// Button_EyeState
	//UPROPERTY(EditDefaultsOnly)


private:
	class UImage *EyeViewImage = nullptr;

	UPROPERTY()
	class UTexture2D* EyeViewTexture2D;
};
