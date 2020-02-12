// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHudWidget.h"
#include "DaphniaPawn.h"
#include "ParallelPhysics/PPSettings.h"
#include "MyPlayerController.h"
#include "Components/Image.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ParallelPhysics/ParallelPhysics.h"
#include "array"
#include "Kismet/GameplayStatics.h"

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
			const int32 EyeTextureSize = ADaphniaPawn::GetInstance()->GetEyeTextureSize();
			for (int32 i = 0; i < (EyeTextureSize * EyeTextureSize); i++)
			{
				RawData[i].A = 255;
			}

			EyeViewTexture2D = UTexture2D::CreateTransient(EyeTextureSize, EyeTextureSize);
			check(EyeViewTexture2D);
			if (EyeViewTexture2D)
			{
				FTexture2DMipMap& Mip = EyeViewTexture2D->PlatformData->Mips[0];
				void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
				FMemory::Memcpy(Data, RawData.GetData(), (EyeTextureSize * EyeTextureSize * 4));
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
	check(s_InstancePtr);
	return s_InstancePtr;
}

void UMyHudWidget::SwitchCameraView()
{
	ADaphniaPawn::GetInstance()->SwitchView();
}

void UMyHudWidget::SwitchToParallelPhysics()
{
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		PPh::SP_EyeState eyeState = std::make_shared<PPh::EyeArray>();
		PPh::EyeArray &eyeArray = *eyeState;
		UGameViewportClient* ViewportClient = World->GetGameViewport();
		if (ViewportClient)
		{
			ViewportClient->bDisableWorldRendering = true;
			
			FIntPoint Pos = ViewportClient->Viewport->GetInitialPositionXY();
			FIntPoint Size = ViewportClient->Viewport->GetSizeXY();
			FVector worldOrigin, worldDirection;
			check(eyeArray.size() == PPh::OBSERVER_EYE_SIZE);
			for (int ii = 0; ii < PPh::OBSERVER_EYE_SIZE; ++ii)
			{
				check(eyeArray[ii].size() == PPh::OBSERVER_EYE_SIZE);
				for (int jj = 0; jj < PPh::OBSERVER_EYE_SIZE; ++jj)
				{
					FVector2D PosFloat;
					PosFloat.X = (float)Pos.X + Size.X * ((float)jj / (PPh::OBSERVER_EYE_SIZE - 1));
					PosFloat.Y = (float)Pos.Y + Size.Y * ((float)ii / (PPh::OBSERVER_EYE_SIZE - 1));

					UGameplayStatics::DeprojectScreenToWorld(AMyPlayerController::GetInstance(), PosFloat, worldOrigin, worldDirection);
					eyeArray[ii][jj] = UPPSettings::ConvertRotationToPPhOrientation(worldDirection);
				}
			}
			FVector pawnLocation = ADaphniaPawn::GetInstance()->GetActorLocation();
			PPh::VectorIntMath position = UPPSettings::ConvertLocationToPPhPosition(pawnLocation);
			PPh::Observer::Init(position, eyeState);
			PPh::ParallelPhysics::GetInstance()->StartSimulation();
		}
	}
}
