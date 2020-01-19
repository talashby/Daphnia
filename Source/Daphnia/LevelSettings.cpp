// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelSettings.h"
#include "../../../../Program Files/Epic Games/UE_4.24/Engine/Source/Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "DaphniaPawn.h"

// **************************** FRoomVolumeSettings *********************************
FRoomVolumeSettings::FRoomVolumeSettings()
{
	GameObjectsNum = 30;
}

// **************************** ALevelSettings *********************************
static ALevelSettings *s_InstancePtr;

// Sets default values
ALevelSettings::ALevelSettings()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ObjectPlaceSize = 100;
	iCurrentDoorToOpen = 0;
	bIsFinished = false;

	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		//ConstructorHelpers::FObjectFinderOptional<UMaterial> ColonyNameMaterial;
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> SphereMesh;
		FConstructorStatics()
			:
			//ColonyNameMaterial(TEXT("Material'/Game/Blueprints/ResourceMaterial.ResourceMaterial'")),
			SphereMesh(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"))
		{}
	};
	static FConstructorStatics ConstructorStatics;
	SphereMesh = ConstructorStatics.SphereMesh.Get();

}

ALevelSettings* ALevelSettings::GetInstance()
{
	checkSlow(s_InstancePtr);
	return s_InstancePtr;
}

// Called when the game starts or when spawned
void ALevelSettings::BeginPlay()
{
	s_InstancePtr = this;
	Super::BeginPlay();
}

// Called every frame
void ALevelSettings::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelSettings::OnGameObjectOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == ADaphniaPawn::GetInstance())
	{
		UStaticMeshComponent *pGameObjectComponent = Cast<UStaticMeshComponent>(OverlappedComp);

		if (pGameObjectComponent)
		{
			for (int ii = 0; ii < GameObjectMaterials.Num(); ++ii)
			{
				if (GameObjectMaterials[ii] == OverlappedComp->GetMaterial(0))
				{
					// todo
				}
			}
		}

		AActor *pActor = OverlappedComp->GetOwner();
		if (pActor)
		{
			pActor->Destroy();
		}
	}
}


