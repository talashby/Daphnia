// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DaphniaGameMode.generated.h"

UCLASS(MinimalAPI)
class ADaphniaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADaphniaGameMode();

	// Begin AActor interface
	virtual void BeginPlay() override;

protected:
	enum class EWidgetZOrder
	{
		Back = 0,
		Modal,
		Hud
	};

	UPROPERTY()
	class UMyHudWidget *HUDWidget = nullptr;
	TSubclassOf<class UMyHudWidget> HUDWidgetClass = nullptr; // blueprint class
};



