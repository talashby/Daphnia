// Fill out your copyright notice in the Description page of Project Settings.


#include "MyHudWidget.h"
#include "DaphniaPawn.h"
#include "ParallelPhysics/PPSettings.h"
#include "MyPlayerController.h"
#include "Components/Image.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ParallelPhysics/AdminTcpClient.h"
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
		check(eyeArray.size() == PPh::GetObserverEyeSize());
		for (int ii = 0; ii < PPh::GetObserverEyeSize(); ++ii)
		{
			check(eyeArray[ii].size() == PPh::GetObserverEyeSize());
			for (int jj = 0; jj < PPh::GetObserverEyeSize(); ++jj)
			{
				FVector2D PosFloat;
				PosFloat.X = (float)Pos.X + Size.X * ((float)jj / (PPh::GetObserverEyeSize() - 1));
				PosFloat.Y = (float)Pos.Y + Size.Y * ((float)ii / (PPh::GetObserverEyeSize() - 1));

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
	if (PPh::ObserverClient::Instance() && PPh::ObserverClient::Instance()->IsSimulationRunning())
	{
		PPh::ObserverClient::Instance()->StopSimulation();
	}
}

void UMyHudWidget::NativeTick(const FGeometry &MyGeometry, float InDeltaTime)
{
	if (PPh::ObserverClient::Instance() && PPh::ObserverClient::Instance()->IsSimulationRunning())
	{
		const auto &otherObserversMap = MyObserver::MyInstance()->GetOtherObserversMap();
		for (const auto &it : otherObserversMap)
		{
			if (it.second.m_actor == nullptr)
			{
				AActor *actor = ALevelSettings::GetInstance()->SpawnOtherObserver();
				MyObserver::MyInstance()->SetOtherObserverActor(it.first, actor);
			}
			AActor *crumb = PPh::AdminUniverse::EtherCellGetCrumbActor(it.second.m_pos);
			if (crumb)
			{
				ALevelSettings::GetInstance()->EatCrumb(crumb);
				PPh::AdminUniverse::EtherCellSetCrumbActor(it.second.m_pos, nullptr);
			}
			it.second.m_actor->SetActorLocation(UPPSettings::ConvertPPhPositionToLocation(it.second.m_pos));
			FRotator orient(it.second.m_latitude, it.second.m_longitude, 0);
			it.second.m_actor->SetActorRotation(orient);
		}

		PPh::VectorInt32Math outPosition;
		uint16_t outMovingProgress;
		int16_t outLatitude, outLongitude;
		bool outIsEatenCrumb = false;
		PPh::VectorInt32Math outEatenCrumbPos;
		PPh::ObserverClient::Instance()->GetStateExtParams(outPosition, outMovingProgress, outLatitude, outLongitude, outIsEatenCrumb);
		FVector location = UPPSettings::ConvertPPhPositionToLocation(outPosition);
		ADaphniaPawn::GetInstance()->SetActorLocation(location);
		FRotator orient(outLatitude, outLongitude, 0);
		//orient.Vector() * outMovingProgress / std::numeric_limits<uint16_t>.max() ;
		ADaphniaPawn::GetInstance()->SetActorRotation(orient);
		if (outIsEatenCrumb)
		{
			outEatenCrumbPos = PPh::ObserverClient::Instance()->GrabEatenCrumbPos();
			ALevelSettings::GetInstance()->EatCrumb(PPh::AdminUniverse::EtherCellGetCrumbActor(outEatenCrumbPos));
			PPh::AdminUniverse::EtherCellSetCrumbActor(outEatenCrumbPos, nullptr);
		}
		ShowPPhStats(outLatitude, outLongitude, outPosition);
		if (pEyeViewImage && pEyeViewImage->IsVisible())
		{
			PPh::SP_EyeColorArray spEyeColorArray = PPh::ObserverClient::Instance()->GrabTexture();
			if (spEyeColorArray)
			{
				const int32 EyeTextureSize = PPh::GetObserverEyeSize();
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

void UMyHudWidget::ShowPPhStats(int16_t latitude, int16_t longitude, const PPh::VectorInt32Math &position)
{
	static int64 lastTime = PPh::GetTimeMs();
	if (PPh::GetTimeMs() - lastTime > 500)
	{
		lastTime = PPh::GetTimeMs();
		if (pTextBlockStats && PPh::ObserverClient::Instance()->IsSimulationRunning())
		{
			uint32_t outQuantumOfTimePerSecond;
			uint32_t outUniverseThreadsNum;
			uint32_t outTickTimeMusAverageUniverseThreadsMin;
			uint32_t outTickTimeMusAverageUniverseThreadsMax;
			uint32_t outTickTimeMusAverageObserverThread;
			uint64_t outClientServerPerformanceRatio;
			uint64_t outServerClientPerformanceRatio;
			PPh::ObserverClient::Instance()->GetStatisticsParams(outQuantumOfTimePerSecond, outUniverseThreadsNum,
				outTickTimeMusAverageUniverseThreadsMin, outTickTimeMusAverageUniverseThreadsMax,
				outTickTimeMusAverageObserverThread, outClientServerPerformanceRatio, outServerClientPerformanceRatio);
			FString sFps = FString("FPS (quantum of time per second): ") + FString::FromInt(outQuantumOfTimePerSecond);
			sFps += "\nUniverse threads count: " + FString::FromInt(outUniverseThreadsNum);
			if (outUniverseThreadsNum)
			{
				sFps += "\nTick time(ms). Observer thread: " + FString::SanitizeFloat(outTickTimeMusAverageObserverThread / 1000.0f);
				sFps += "\nTick time(ms). Fastest universe thread: " + FString::SanitizeFloat(outTickTimeMusAverageUniverseThreadsMin / 1000.0f);
				sFps += "\nTick time(ms). Slowest universe thread: " + FString::SanitizeFloat(outTickTimeMusAverageUniverseThreadsMax / 1000.0f);
			}
			sFps += "\nClient-Server performance ratio: " + FString::SanitizeFloat(outClientServerPerformanceRatio / 1000.0f);
			sFps += "\nServer-Client performance ratio: " + FString::SanitizeFloat(outServerClientPerformanceRatio / 1000.0f);
			sFps += FString("\nPosition: (") + FString::FromInt(position.m_posX) + ", " + FString::FromInt(position.m_posY) + ", " +
				FString::FromInt(position.m_posZ) + ")";
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
	if (PPh::ObserverClient::Instance() && PPh::ObserverClient::Instance()->IsSimulationRunning())
	{
		PPh::ObserverClient::Instance()->StopSimulation();
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
			PPh::ObserverClient::Init(new MyObserver());
			PPh::ObserverClient::Instance()->StartSimulation();
			uint64_t observerId = PPh::ObserverClient::Instance()->GetLastObserverID();
			PPh::AdminTcp::Connect();
			PPh::AdminTcp::RegisterAdminObserver(observerId);
			PPh::AdminTcp::Disconnect();
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
