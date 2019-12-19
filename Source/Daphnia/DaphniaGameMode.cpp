// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DaphniaGameMode.h"
#include "DaphniaPawn.h"

ADaphniaGameMode::ADaphniaGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ADaphniaPawn::StaticClass();
}
