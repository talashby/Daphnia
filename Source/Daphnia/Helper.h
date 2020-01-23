// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"
#include "EngineUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGeneral, Log, All);

namespace LogHelper
{
	bool CheckLogLevel(int iLogSeverityLevel);
}

namespace MyHelper
{
	// empty name for first actor
	template <typename T> bool GetActorFromScene(UWorld *pWorld, FString sActorName, T* &pOutActor)
	{
		for (TActorIterator<T> ActorItr(pWorld); ActorItr; ++ActorItr)
		{
			FString SceneActorName = ActorItr->GetName();
			if (!SceneActorName.Compare(sActorName) || sActorName.IsEmpty())
			{
				pOutActor = *ActorItr;
				return true;
			}
		}
		return false;
	}
}
