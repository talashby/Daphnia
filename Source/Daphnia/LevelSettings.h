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
	class ATriggerVolume *TriggerVolume = nullptr;

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 GameObjectsNum;
};

UCLASS()
class DAPHNIA_API ALevelSettings : public AActor, public TSharedFromThis<ALevelSettings>
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALevelSettings();

	~ALevelSettings();

	static ALevelSettings* GetInstance();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PlayCrumbSound() const;
	void EatCrumb(AActor *actor);
	//UFUNCTION()
	//void OnGameObjectOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	//								int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void OnConstruction(const FTransform& Transform) override;

	FBox GetUniverseBoundingBox() const;

	const TArray<class UMaterialInstance*>& GetGameObjectMaterials() const;

	AActor* SpawnOtherObserver();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
//	class AAmbientSound* SpawnAmbientSound();
	void GenerateItems(const FRoomVolumeSettings &Settings);
	AActor*  SpawnCrumb(FVector pos, int32 crumbMaterialNum);
	void OnMapLoaded();
	void LoadCrumbsFromServer();
// ****************************************************
// **** Settings

// ****************************************************
// **** pickupable game objects

	UPROPERTY()
	UStaticMesh *SphereMesh;

	UPROPERTY(VisibleAnywhere, Category = "Variables")
	TArray<UStaticMesh*> GameObjects;

	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<class UMaterialInstance*> GameObjectMaterials;

	// rooms for placing game object
	UPROPERTY(EditAnywhere, Category = "Settings")
	TArray<FRoomVolumeSettings> RoomVolumeSettings;

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 ObjectPlaceSize = 100;

	UPROPERTY(EditAnywhere, Category = "Settings")
	class USoundBase *EatCrumbSound;


	// ************************************************************
	// ************** other
	// every crumb sing song...
	UPROPERTY(EditAnywhere, Category = "Settings")
	class USoundBase *CrumbMusic;

	UPROPERTY()
	class UPPSettings *PPSettings = nullptr;
	TSubclassOf<class UPPSettings> PPSettingsClass = nullptr; // blueprint class

	UPROPERTY()
	UStaticMesh *m_observerMesh;

	UPROPERTY()
	UMaterialInterface *m_observerMaterial;

	int iCurrentDoorToOpen;
	bool bIsFinished;
};
