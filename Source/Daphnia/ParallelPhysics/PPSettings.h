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

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 ObjectPlaceSize = 110;

	/** Set university size from cpp code. */
	UPROPERTY(EditAnywhere, NoClear, BlueprintReadOnly, Category = "Settings")
	bool bUseCppUniversitySize = false;

	/** University size. */
	UPROPERTY(EditAnywhere, NoClear, BlueprintReadOnly, Category = "Settings")
	FBox UniversitySize = FBox(EForceInit::ForceInit);
};
