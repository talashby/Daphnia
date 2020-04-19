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
#include "LevelSettings.h"
#include "ParallelPhysics/PPhHelpers.h"
#include "limits"

static UMyHudWidget* s_InstancePtr;


PPh::SP_EyeState GetPawnEyeState(UWorld* World)
{
	PPh::SP_EyeState eyeState = std::make_shared<PPh::EyeArray>();
	PPh::EyeArray &eyeArray = *eyeState;
	UGameViewportClient* ViewportClient = World->GetGameViewport();
	if (ViewportClient)
	{
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
	}

	return eyeState;
}

UMyHudWidget::UMyHudWidget() : UUserWidget(FObjectInitializer())
{
	bIsFocusable = false;
}

void UMyHudWidget::NativeOnInitialized()
{
	s_InstancePtr = this;
	pEyeViewImage = WidgetTree->FindWidget<UImage>(TEXT("EyeCaptureImg"));
	pTextBlockStats = WidgetTree->FindWidget<UTextBlock>(TEXT("TextBlock_Stats"));
}

void UMyHudWidget::NativeDestruct()
{
	if (PPh::ParallelPhysics::GetInstance() && PPh::ParallelPhysics::GetInstance()->IsSimulationRunning())
	{
		PPh::ParallelPhysics::GetInstance()->StopSimulation();
	}
}

void UMyHudWidget::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
	if (!PPh::ParallelPhysics::GetInstance())
	{
		return;
	}

	if (PPh::ParallelPhysics::GetInstance()->IsSimulationRunning() && PPh::Observer::GetInstance())
	{
		PPh::VectorInt32Math outPosition;
		uint16_t outMovingProgress;
		int16_t outLatitude, outLongitude;
		PPh::VectorInt32Math outEatenCrumbPos;
		PPh::Observer::GetInstance()->GetStateParams(outPosition, outMovingProgress, outLatitude, outLongitude, outEatenCrumbPos);
		FVector location = UPPSettings::ConvertPPhPositionToLocation(outPosition);
		ADaphniaPawn::GetInstance()->SetActorLocation(location);
		FRotator orient(outLatitude, outLongitude, 0);
		//orient.Vector() * outMovingProgress / std::numeric_limits<uint16_t>.max() ;
		ADaphniaPawn::GetInstance()->SetActorRotation(orient);
		if (outEatenCrumbPos != PPh::VectorInt32Math::ZeroVector)
		{
			ALevelSettings::GetInstance()->EatCrumb(PPh::ParallelPhysics::EtherCellGetCrumbActor(outEatenCrumbPos));
		}
		ShowPPhStats(outLatitude, outLongitude);
		if (pEyeViewImage && pEyeViewImage->IsVisible())
		{
			/*if (m_PawnRotation != ADaphniaPawn::GetInstance()->GetActorRotation())
			{
				m_PawnRotation = ADaphniaPawn::GetInstance()->GetActorRotation();
				PPh::SP_EyeState eyeState = GetPawnEyeState(GetWorld());
				PPh::Observer::GetInstance()->ChangeOrientation(eyeState);
			}
			*/
			PPh::SP_EyeColorArray spEyeColorArray = PPh::Observer::GetInstance()->GrabTexture();
			if (spEyeColorArray)
			{
				const int32 EyeTextureSize = PPh::OBSERVER_EYE_SIZE;
				if (!pEyeViewTexture2D)
				{
					pEyeViewTexture2D = UTexture2D::CreateTransient(EyeTextureSize, EyeTextureSize);
					check(pEyeViewTexture2D);
				}
				if (pEyeViewTexture2D)
				{
					FTexture2DMipMap& Mip = pEyeViewTexture2D->PlatformData->Mips[0];
					void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
					FMemory::Memcpy(Data, &(*spEyeColorArray)[0][0], (EyeTextureSize * EyeTextureSize * 4));
					//FMemory::Memset(Data, 255, 256);
					Mip.BulkData.Unlock();
					pEyeViewTexture2D->UpdateResource();
				}

				pEyeViewImage->SetBrushFromTexture(pEyeViewTexture2D);
			}
		}
		/*auto RenderTarget2D = ADaphniaPawn::GetInstance()->GetEyeRenderTarget2D();
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
		}*/
	}

	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMyHudWidget::ShowPPhStats(int16_t latitude, int16_t longitude)
{
	static int64 lastTime = PPh::GetTimeMs();
	if (PPh::GetTimeMs() - lastTime > 500)
	{
		lastTime = PPh::GetTimeMs();
		if (pTextBlockStats && PPh::ParallelPhysics::GetInstance()->IsSimulationRunning())
		{
			FString sFps = FString("FPS (quantum of time per second): ") + FString::FromInt(PPh::ParallelPhysics::GetFPS());
			uint32_t universeThreadsNum = PPh::ParallelPhysics::GetUniverseThreadsNum();
			sFps += "\nUniverse threads count: " + FString::FromInt(PPh::ParallelPhysics::GetUniverseThreadsNum());
			if (universeThreadsNum)
			{
				sFps += "\nTick time(ms). Observer thread: " + FString::SanitizeFloat(PPh::ParallelPhysics::GetTickTimeMusObserverThread()/1000.0f);
				sFps += "\nTick time(ms). Fastest universe thread: " + FString::SanitizeFloat(PPh::ParallelPhysics::GetTickTimeMusUniverseThreadsMin() / 1000.0f);
				sFps += "\nTick time(ms). Slowest universe thread: " + FString::SanitizeFloat(PPh::ParallelPhysics::GetTickTimeMusUniverseThreadsMax() / 1000.0f);
			}
			sFps += FString("\nLattitude: ") + FString::FromInt(latitude);
			sFps += FString("\nLongitude: ") + FString::FromInt(longitude);
			pTextBlockStats->SetText(FText::FromString(sFps));
		}
	}
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
	UWidget *BoxStats = WidgetTree->FindWidget<UWidget>(TEXT("Border_Stats"));
	if (PPh::ParallelPhysics::GetInstance()->IsSimulationRunning())
	{
		PPh::ParallelPhysics::GetInstance()->StopSimulation();
		if (BoxStats)
		{
			BoxStats->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		UWorld* World = GetWorld();
		if (World && World->IsGameWorld())
		{
			DisableRenderCheckBoxPressed();
			PPh::SP_EyeState eyeState;
			eyeState = GetPawnEyeState(World);
			FVector pawnLocation = ADaphniaPawn::GetInstance()->GetActorLocation();
//			PPh::VectorInt32Math position = UPPSettings::ConvertLocationToPPhPosition(pawnLocation);
			PPh::Observer::Init();
			PPh::ParallelPhysics::GetInstance()->StartSimulation();
//			m_PawnRotation = ADaphniaPawn::GetInstance()->GetActorRotation();
//			m_ObserverPos = PPh::Observer::GetInstance()->GetPosition();

			if (BoxStats)
			{
				BoxStats->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void UMyHudWidget::DisableRenderCheckBoxPressed()
{
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		UGameViewportClient* ViewportClient = World->GetGameViewport();
		if (ViewportClient)
		{
			ViewportClient->bDisableWorldRendering = m_bDisableRenderCheckBox;
		}
	}
}
