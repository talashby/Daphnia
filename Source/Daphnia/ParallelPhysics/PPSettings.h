// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PPSettings.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class UPPSettings : public UObject
{
	GENERATED_BODY()

	UPPSettings();

public:

	void Init(UWorld *World);
	void ConvertGeometry(UWorld *World);

private:

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 UniverseEtherCellSize = 100;

	/** Set universe size from cpp code. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bUseCppUniverseSize = false;

	/** Universe size. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	FBox UniverseBox = FBox(EForceInit::ForceInit);
};
