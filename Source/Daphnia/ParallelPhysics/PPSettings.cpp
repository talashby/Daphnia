// Fill out your copyright notice in the Description page of Project Settings.


#include "PPSettings.h"
#include "../LevelSettings.h"

UPPSettings::UPPSettings()
{
	int ttt = 0;
}

void UPPSettings::Init(UWorld *World)
{
	check(World);

	ObjectPlaceSize = 111;

	if (bUseCppUniversitySize)
	{
		FVector vecOrigin, vecBoxExtent;
		ALevelSettings::GetInstance()->GetUniversityBounds(vecOrigin, vecBoxExtent);
		UniversitySize = FBox::BuildAABB(vecBoxExtent, vecOrigin);
	}
}
