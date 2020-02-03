// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "DaphniaPawn.h"
#include "Engine/TriggerVolume.h"
#include "vector"
#include "Engine/StaticMeshActor.h"
#include "random"
#include "Sound/AmbientSound.h"
#include "Kismet/GameplayStatics.h"
#include "Helper.h"
#include "Components/AudioComponent.h"
#include "ParallelPhysics/PPSettings.h"

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

	{
		static ConstructorHelpers::FObjectFinder<UClass> ItemBlueprint(TEXT("Blueprint'/Game/Blueprints/BP_PPSettings.BP_PPSettings_C'"));
		if (ItemBlueprint.Object)
		{
			PPSettingsClass = ItemBlueprint.Object;
			PPSettings = Cast<UPPSettings>(PPSettingsClass->GetDefaultObject());
		}
	}
}

void ALevelSettings::OnConstruction(const FTransform& Transform)
{
	OnMapLoaded();
}

void ALevelSettings::OnMapLoaded()
{
	UWorld* World = GetWorld();
	if (World && PPSettings)
	{
		PPSettings->ObjectPlaceSize = 111;

		if (PPSettings->bUseCppUniversitySize && 0 < RoomVolumeSettings.Num())
		{
			FVector vecOrigin, vecBoxExtent;
			RoomVolumeSettings[0].TriggerVolume->GetActorBounds(false, vecOrigin, vecBoxExtent);
			PPSettings->UniversitySize = FBox::BuildAABB(vecBoxExtent, vecOrigin);
		}
	}
}

ALevelSettings::~ALevelSettings()
{
	//FEditorDelegates::OnMapOpened.RemoveAll(this);
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

	for (const auto & Settings : RoomVolumeSettings)
	{
		GenerateItems(Settings);
	}
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
			const TArray<UActorComponent *>& Comps = pActor->GetInstanceComponents();
			for (auto& Comp : Comps)
			{
				UAudioComponent* Snd = Cast<UAudioComponent>(Comp);
				if (Snd)
				{
					Snd->Stop();
				}
			}
			pActor->Destroy();
			UGameplayStatics::PlaySound2D(GetWorld(), EatCrumbSound);
		}
	}
}

std::random_device rd;
std::mt19937 e1(rd());

int32 Rand32(int32 iRandMax) // from [0; iRandMax-1]
{
	checkSlow(iRandMax > 0);
	std::uniform_int_distribution<int32> dist(0, iRandMax-1);
	return dist(e1);
}


std::random_device rd64;
std::mt19937_64 e2(rd64());

int64 Rand64(int64 iRandMax) // from [0; iRandMax-1]
{
	checkSlow(iRandMax > 0);
	std::uniform_int_distribution<int64> dist(0, iRandMax-1);
	return dist(e2);
}

/*AAmbientSound* ALevelSettings::SpawnAmbientSound()
{
	// Spawn!
	AAmbientSound *SoundObject = GetWorld()->SpawnActor<AAmbientSound>(AAmbientSound::StaticClass());
	UAudioComponent* AudioComponent = SoundObject->GetAudioComponent();
	if (AudioComponent)
	{
		//AudioComponent->Set
	}
	return SoundObject;
}*/

void ALevelSettings::GenerateItems(const FRoomVolumeSettings &Settings)
{
	int32 MaterialsNum = GameObjectMaterials.Num();
	check(MaterialsNum);
	if (!MaterialsNum)
	{
		return;
	}
	FVector vecOrigin, vecBoxExtent;

	Settings.TriggerVolume->GetActorBounds(false, vecOrigin, vecBoxExtent);

	// makes vecOrigin points to the corner
	vecOrigin.X -= vecBoxExtent.X;
	vecOrigin.Y -= vecBoxExtent.Y;
	vecOrigin.Z -= vecBoxExtent.Z;

	constexpr int32 NumObjectsBorder = 2;
	int32 iPlacesForObjectsX = FMath::RoundToInt(vecBoxExtent.X * 2) / ObjectPlaceSize - NumObjectsBorder*2;
	int32 iShiftX = (FMath::RoundToInt(vecBoxExtent.X * 2) % ObjectPlaceSize) / 2 + NumObjectsBorder*ObjectPlaceSize;
	int32 iPlacesForObjectsY = FMath::RoundToInt(vecBoxExtent.Y * 2) / ObjectPlaceSize - NumObjectsBorder * 2;
	int32 iShiftY = (FMath::RoundToInt(vecBoxExtent.Y * 2) % ObjectPlaceSize) / 2 + NumObjectsBorder*ObjectPlaceSize;
	int32 iPlacesForObjectsZ = FMath::RoundToInt(vecBoxExtent.Z * 2) / ObjectPlaceSize - NumObjectsBorder * 2;
	int32 iShiftZ = (FMath::RoundToInt(vecBoxExtent.Z * 2) % ObjectPlaceSize) / 2 + NumObjectsBorder*ObjectPlaceSize;

	if (static_cast<int64>(iPlacesForObjectsX) * iPlacesForObjectsY * iPlacesForObjectsZ > std::numeric_limits<int32>::max())
	{
		check(false);
		return;
	}
	std::vector<int32> ResultArray(Settings.GameObjectsNum);
	{	// The Knuth algorithm to find random unique values
		int32 M = Settings.GameObjectsNum;
		int32 N = iPlacesForObjectsX * iPlacesForObjectsY * iPlacesForObjectsZ;
		int32 in, im;

		im = 0;

		for (in = 0; in < N && im < M; ++in) {
			int32 rn = N - in;
			int32 rm = M - im;
			if (Rand32(rn) < rm)
				/* Take it */
				ResultArray[im++] = in;
		}

		check(im == M);
	}

	// place objects
	int32 iKeyPlace = ResultArray[rand() % ResultArray.size()];
	for (int32 iRandPlaceNumber : ResultArray)
	{
		int32 XandY = iRandPlaceNumber % (iPlacesForObjectsX * iPlacesForObjectsY);
		int32 iPlaceX = vecOrigin.X + iShiftX + (XandY % iPlacesForObjectsX) * ObjectPlaceSize;
		int32 iPlaceY = vecOrigin.Y + iShiftY + (XandY / iPlacesForObjectsX) * ObjectPlaceSize;
		int32 iPlaceZ = vecOrigin.Z + iShiftZ + (iRandPlaceNumber / (iPlacesForObjectsX * iPlacesForObjectsY)) * ObjectPlaceSize;

		// Spawn!
		AStaticMeshActor *pGameObject = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector(iPlaceX, iPlaceY, iPlaceZ), FRotator(0, 0, 0));
		pGameObject->SetMobility(EComponentMobility::Static);
		TArray<UStaticMeshComponent*> Comps;
		pGameObject->GetComponents(Comps);
		if (Comps.Num() > 0)
		{
			UStaticMeshComponent* FoundComp = Comps[0];
			FoundComp->SetStaticMesh(SphereMesh);
			int32 iRnd = rand() % MaterialsNum;

			FoundComp->SetMaterial(0, GameObjectMaterials[iRnd]);
			FoundComp->SetCastShadow(false);
			FoundComp->SetGenerateOverlapEvents(true);
			FCollisionResponseContainer collision_response;
			collision_response.SetAllChannels(ECollisionResponse::ECR_Overlap);
			FoundComp->SetCollisionResponseToChannels(collision_response);
			FoundComp->OnComponentBeginOverlap.AddUniqueDynamic(ALevelSettings::GetInstance(), &ALevelSettings::OnGameObjectOverlapBegin);
			{ // add sound component
				if (CrumbMusic)
				{
					UAudioComponent* NewAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CrumbMusic, pGameObject->GetActorLocation());
					pGameObject->AddInstanceComponent(NewAudioComponent);
					NewAudioComponent->Play();
				}

				/*UAudioComponent* AudioComponent = AmbientSoundCrumb->GetAudioComponent();
				UAudioComponent* NewAudioComponent = DuplicateObject(AudioComponent, pGameObject);
				NewAudioComponent->SetupAttachment(FoundComp);
				NewAudioComponent->SetRelativeLocation(FVector());
				NewAudioComponent->Play();
				FVector Location = NewAudioComponent->GetComponentTransform().GetLocation();
				int eee = 0;*/
			}
		}

		TSet<AActor*> aOverlapActors;
		pGameObject->UpdateOverlaps();
		pGameObject->GetOverlappingActors(aOverlapActors);
		if (aOverlapActors.Num())
		{
			pGameObject->Destroy();
		}
	}
}


