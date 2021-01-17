// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PPhHelpers.h"
#include "ObserverClient.h"
#include <map>

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
	static UPPSettings* GetInstance();
	void InitParallelPhysics();
	void ConvertGeometry(UWorld *World);
	static PPh::VectorInt32Math ConvertLocationToPPhPosition(const FVector &Location);
	static FVector ConvertPPhPositionToLocation(const PPh::VectorInt32Math &pos);
	static PPh::OrientationVectorMath ConvertRotationToPPhOrientation(const FVector &orientationVector);
	static PPh::OrientationVectorMath ConvertRotationToPPhOrientation(const FRotator &Rotator);
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 UniverseEtherCellSize = 100;

private:

	/** Set universe size from cpp code. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bUseCppUniverseSize = false;

	/** Universe size. */
	UPROPERTY(EditAnywhere, Category = "Settings")
	FBox UniverseBox = FBox(EForceInit::ForceInit);
};

struct ObserverData
{
	PPh::VectorInt32Math m_pos;
	int16_t m_latitude;
	int16_t m_longitude;
	AActor *m_actor = nullptr;
};

typedef std::map<uint64_t, ObserverData> OtherObserversMap;

class MyObserver : public PPh::ObserverClient
{
public:
	MyObserver();
	~MyObserver();

	static MyObserver* MyInstance();
	const OtherObserversMap& GetOtherObserversMap() const;
	void SetOtherObserverActor(uint64_t observerId, AActor *actor);

protected:
	void HandleReceivedMessage(const char *buffer) override;

private:
	OtherObserversMap m_otherObserversMap;
};