// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHudWidget.h"
#include "DaphniaPawn.h"


#include "MyPlayerController.h"
#include "Components/Image.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Materials/MaterialInstanceDynamic.h"

static UMyHudWidget* s_InstancePtr;


UMyHudWidget::UMyHudWidget() : UUserWidget(FObjectInitializer())
{
	bIsFocusable = false;
}

void UMyHudWidget::NativeOnInitialized()
{
	s_InstancePtr = this;
	EyeViewImage = WidgetTree->FindWidget<UImage>(TEXT("EyeCaptureImg"));
}

void UMyHudWidget::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
	if (EyeViewImage && EyeViewImage->IsVisible())
	{
		auto RenderTarget2D = ADaphniaPawn::Instance()->GetEyeRenderTarget2D();
		if (RenderTarget2D)
		{
			EyeViewImage->SetBrushFromTexture(nullptr);
			EyeViewTexture2D = RenderTarget2D->ConstructTexture2D(this, FString("Snapshot"), EObjectFlags::RF_NoFlags, CTF_ForceOpaque | CTF_Compress);
			checkSlow(EyeViewTexture2D);
			EyeViewImage->SetBrushFromTexture(EyeViewTexture2D);
		}
	}

	Super::NativeTick(MyGeometry, InDeltaTime);
}

UMyHudWidget* UMyHudWidget::Instance()
{
	checkSlow(s_InstancePtr);
	return s_InstancePtr;
}

void UMyHudWidget::SwitchCameraView()
{
	ADaphniaPawn::Instance()->SwitchView();
}
