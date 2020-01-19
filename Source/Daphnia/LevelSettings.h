// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelSettings.generated.h"

USTRUCT()
struct FRoomVolumeSettings
{
	GENERATED_BODY()
	FRoomVolumeSettings();

	UPROPERTY(EditAnywhere, Category = "Settings")
	class ATriggerVolume *TriggerVolume;

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 GameObjectsNum;
};

UCLASS()
class ALevelSettings : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelSettings();

	static ALevelSettings* GetInstance();

	// Called every frame
	virtual void Tick(float DeltaTime) override;



	UFUNCTION()
	void OnGameObjectOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
									int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	// ****************************************************
	// **** Settings


	// pickupable game objects
	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<class UMaterial*> GameObjectMaterials;

	// settings for placing game object
	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<FRoomVolumeSettings> RoomVolumeSettings;

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 ObjectPlaceSize;


	// ****************************************************
	// **** game objects initialized in code (visible in editor)
	UPROPERTY(VisibleAnywhere, Category = "Variables")
	TArray<UStaticMesh*> GameObjects;

	// ************************************************************
	// ************** other
	UPROPERTY()
	UStaticMesh *SphereMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	int iCurrentDoorToOpen;
	bool bIsFinished;
};
