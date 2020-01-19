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
		auto RenderTarget2D = ADaphniaPawn::GetInstance()->GetEyeRenderTarget2D();
		if (RenderTarget2D)
		{
			EyeViewImage->SetBrushFromTexture(nullptr);
			EyeViewTexture2D = nullptr;

			FTextureRenderTargetResource* Resource = RenderTarget2D->GameThread_GetRenderTargetResource();
			TArray<FColor> RawData;
			Resource->ReadPixels(RawData);
			for (int32 i = 0; i < (32 * 32); i++)
			{
				RawData[i].A = 255;
			}

			EyeViewTexture2D = UTexture2D::CreateTransient(32, 32);
			check(EyeViewTexture2D);
			if (EyeViewTexture2D)
			{
				FTexture2DMipMap& Mip = EyeViewTexture2D->PlatformData->Mips[0];
				void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(Data, RawData.GetData(), (32 * 32 * 4));
				Mip.BulkData.Unlock();
				EyeViewTexture2D->UpdateResource();
			}
			
			EyeViewImage->SetBrushFromTexture(EyeViewTexture2D);
		}
	}

	Super::NativeTick(MyGeometry, InDeltaTime);
}

UMyHudWidget* UMyHudWidget::GetInstance()
{
	checkSlow(s_InstancePtr);
	return s_InstancePtr;
}

void UMyHudWidget::SwitchCameraView()
{
	ADaphniaPawn::GetInstance()->SwitchView();
}
