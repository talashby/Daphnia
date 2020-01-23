// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Helper.h"

DEFINE_LOG_CATEGORY(LogGeneral)

namespace LogHelper
{
	bool CheckLogLevel(int iLogSeverityLevel)
	{
#if NO_LOGGING
		return false;
#else
		if ((iLogSeverityLevel & ELogVerbosity::VerbosityMask) <= ELogVerbosity::COMPILED_IN_MINIMUM_VERBOSITY && (ELogVerbosity::Warning & ELogVerbosity::VerbosityMask) <= FLogCategoryLogGeneral::CompileTimeVerbosity)
		{
			return true;
		}
		return false;
#endif
	}
}
