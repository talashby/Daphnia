// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DaphniaGameMode.h"
#include "DaphniaPawn.h"
#include "MyPlayerController.h"
#include "MyHudWidget.h"
#include "UObject/ConstructorHelpers.h"

ADaphniaGameMode::ADaphniaGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ADaphniaPawn::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();

	{
		static ConstructorHelpers::FObjectFinder<UClass> ItemBlueprint(TEXT("WidgetBlueprint'/Game/Blueprints/Widgets/BP_MyHudWidget.BP_MyHudWidget_C'"));
		if (ItemBlueprint.Object)
		{
			HUDWidgetClass = ItemBlueprint.Object;
		}
	}
}

void ADaphniaGameMode::BeginPlay()
{
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UMyHudWidget>(GetWorld(), HUDWidgetClass);
		HUDWidget->AddToViewport(static_cast<int32>(EWidgetZOrder::Hud));
	}
}