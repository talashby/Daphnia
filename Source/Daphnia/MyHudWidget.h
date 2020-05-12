// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ParallelPhysics/PPhHelpers.h"

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
	void NativeDestruct() override;
	void NativeTick(const FGeometry &MyGeometry, float InDeltaTime) override;

	// Button_SwitchCamera
	UFUNCTION(BlueprintCallable, Category = "BlueprintCallable")
	void SwitchCameraView();

	// Button_ParallelPhysics
	UFUNCTION(BlueprintCallable, Category = "BlueprintCallable")
	void SwitchToParallelPhysics();

	// Button_ParallelPhysics
	UFUNCTION(BlueprintCallable, Category = "BlueprintCallable")
	void DisableRenderCheckBoxPressed();

	UPROPERTY(Category = Menu, VisibleDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool m_bDisableRenderCheckBox = false;

private:
	void ShowPPhStats(int16_t latitude, int16_t longitude, const PPh::VectorInt32Math &position);
	AActor* SpawnObserverMesh();

	class UImage *pEyeViewImage = nullptr;
	class UTextBlock *pTextBlockStats = nullptr;

	UPROPERTY()
	class UTexture2D* pEyeViewTexture2D = nullptr;

	FRotator m_PawnRotation;
	PPh::VectorInt32Math m_ObserverPos;
};
