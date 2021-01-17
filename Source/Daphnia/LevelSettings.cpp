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
#include "ParallelPhysics/ObserverClient.h"
#include "ParallelPhysics/AdminTcpClient.h"
#include "Engine/Engine.h"
#include "ParallelPhysics/AdminProtocol.h"

// **************************** FRoomVolumeSettings *********************************
FRoomVolumeSettings::FRoomVolumeSettings()
{
	GameObjectsNum = 30;
}

// **************************** ALevelSettings *********************************
static ALevelSettings *s_InstancePtr = nullptr;

// Sets default values
ALevelSettings::ALevelSettings()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> meshAsset(TEXT("StaticMesh'/Game/Flying/Meshes/UFO.UFO'"));
		check(meshAsset.Object);
		m_observerMesh = meshAsset.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> materialAsset(TEXT("MaterialInstanceConstant'/Game/Flying/Meshes/UFOMaterial.UFOMaterial'"));
		check(materialAsset.Object);
		m_observerMaterial = materialAsset.Object;
	}
}

void ALevelSettings::OnConstruction(const FTransform& Transform)
{
	// Helper!
	FString ActorName("BP_LevelSettings_5");
	if (0 == ActorName.Compare(GetName())) // insure it is special object in the World not template!
	{ // Used to set ParallelPhysics parameters after editor loaded map
		OnMapLoaded(); // set parameters just editor loaded map so we can see it in the editor
	}
}

FBox ALevelSettings::GetUniverseBoundingBox() const
{
	FBox Box;
	Box.Init();
	if (0 < RoomVolumeSettings.Num())
	{
		Box = RoomVolumeSettings[0].TriggerVolume->GetComponentsBoundingBox();
	}
	else
	{
		check(false);
	}
	return Box;
}

const TArray<class UMaterialInstance*>& ALevelSettings::GetGameObjectMaterials() const
{
	return GameObjectMaterials;
}

void ALevelSettings::OnMapLoaded()
{
	s_InstancePtr = this;
	UWorld* World = GetWorld();
	if (World && PPSettings)
	{
		PPSettings->Init(World);
	}
}

void ALevelSettings::LoadCrumbsFromServer()
{

	PPh::AdminTcp::LoadCrumbs();
}

ALevelSettings::~ALevelSettings()
{
	//FEditorDelegates::OnMapOpened.RemoveAll(this);
}

ALevelSettings* ALevelSettings::GetInstance()
{
	check(s_InstancePtr);
	return s_InstancePtr;
}

// Called when the game starts or when spawned
void ALevelSettings::BeginPlay()
{
	Super::BeginPlay();

	OnMapLoaded();

	bool bSaveUniverseToDisk = false;

	if (bSaveUniverseToDisk)
	{
		PPSettings->InitParallelPhysics();
		for (const auto & Settings : RoomVolumeSettings)
		{
			GenerateItems(Settings);
		}
		UWorld* World = GetWorld();
		if (World && PPSettings)
		{
			PPSettings->ConvertGeometry(World); // used to store universe to disc
		}
	}
	else
	{
		bool bResultConnect = PPh::AdminTcp::Connect();
		if (bResultConnect)
		{
			uint32_t serverVersion = 0;
			bool bResultCheckVersion = PPh::AdminTcp::CheckVersion(serverVersion);
			if (bResultCheckVersion)
			{
				PPSettings->InitParallelPhysics();
				LoadCrumbsFromServer();
				PPh::AdminTcp::Disconnect();
				while (true)
				{
					PPh::VectorInt32Math outCrumbPos;
					PPh::EtherColor outCrumbColor;
					bool bResult = PPh::AdminUniverse::GetNextCrumb(outCrumbPos, outCrumbColor);
					if (!bResult)
					{
						break;
					}
					FVector location = UPPSettings::ConvertPPhPositionToLocation(outCrumbPos);
					int32 UniverseEtherCellSize = UPPSettings::GetInstance()->UniverseEtherCellSize;
					float shift = UniverseEtherCellSize * (PPh::AdminUniverse::GetUniverseScale() - 1) / (PPh::AdminUniverse::GetUniverseScale() * 2.0f);
					location += FVector(shift, shift, shift);
					//static std::array<FColor, 4> Colors = { FColor::Green, FColor::Yellow, FColor::Red, FColor::Blue };
					int32 materialNum = 0;
					if (outCrumbColor.m_colorR == 255 && outCrumbColor.m_colorG == 255)
					{
						materialNum = 1;
					}
					else if (outCrumbColor.m_colorG == 255)
					{
						materialNum = 0;
					}
					else if (outCrumbColor.m_colorR == 255)
					{
						materialNum = 2;
					}
					else if (outCrumbColor.m_colorB == 255)
					{
						materialNum = 3;
					}
					else
					{
						check(false);
					}
					AActor *crumb = SpawnCrumb(location, materialNum);
					PPh::AdminUniverse::EtherCellSetCrumbActor(outCrumbPos, crumb);
				}
			}
			else
			{
				if (!serverVersion)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error. Admin socket CheckVersion timeout.");
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error. Admin protocol wrong version. Client is: " +
						FString::FromInt(PPh::ADMIN_PROTOCOL_VERSION) + ". Server is: " + FString::FromInt(serverVersion) + ".");
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Error. Can't connect to the server.");
		}
	}
}

// Called every frame
void ALevelSettings::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelSettings::PlayCrumbSound() const
{
	UGameplayStatics::PlaySound2D(GetWorld(), EatCrumbSound);
}

void ALevelSettings::EatCrumb(AActor *actor)
{
	if (actor)
	{
		const TArray<UActorComponent *>& Comps = actor->GetInstanceComponents();
		for (auto& Comp : Comps)
		{
			UAudioComponent* Snd = Cast<UAudioComponent>(Comp);
			if (Snd)
			{
				Snd->Stop();
			}
		}
		actor->Destroy();
		//PlayCrumbSound();

		UAudioComponent* NewAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), EatCrumbSound, actor->GetActorLocation());
		NewAudioComponent->Play();
	}
}

/*void ALevelSettings::OnGameObjectOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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
			PlayCrumbSound();
		}
	}
}*/

std::random_device rd;
std::mt19937 e1(rd());

int32 Rand32(int32 iRandMax) // from [0; iRandMax-1]
{
	check(iRandMax > 0);
	std::uniform_int_distribution<int32> dist(0, iRandMax-1);
	return dist(e1);
}


std::random_device rd64;
std::mt19937_64 e2(rd64());

int64 Rand64(int64 iRandMax) // from [0; iRandMax-1]
{
	check(iRandMax > 0);
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

	FBox VolumeBox = Settings.TriggerVolume->GetComponentsBoundingBox();
	FVector VolumeBoxSize = VolumeBox.GetSize();

	constexpr int32 NumObjectsBorder = 2;
	int32 iPlacesForObjectsX = FMath::RoundToInt(VolumeBoxSize.X) / ObjectPlaceSize - NumObjectsBorder*2;
	int32 iShiftX = 50 + (FMath::RoundToInt(VolumeBoxSize.X) % ObjectPlaceSize) / 2 + NumObjectsBorder*ObjectPlaceSize;
	int32 iPlacesForObjectsY = FMath::RoundToInt(VolumeBoxSize.Y) / ObjectPlaceSize - NumObjectsBorder * 2;
	int32 iShiftY = 50 + (FMath::RoundToInt(VolumeBoxSize.Y) % ObjectPlaceSize) / 2 + NumObjectsBorder*ObjectPlaceSize;
	int32 iPlacesForObjectsZ = FMath::RoundToInt(VolumeBoxSize.Z) / ObjectPlaceSize - NumObjectsBorder * 2;
	int32 iShiftZ = 50 + (FMath::RoundToInt(VolumeBoxSize.Z) % ObjectPlaceSize) / 2 + NumObjectsBorder*ObjectPlaceSize;

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
	for (int32 iRandPlaceNumber : ResultArray)
	{
		int32 XandY = iRandPlaceNumber % (iPlacesForObjectsX * iPlacesForObjectsY);
		int32 iPlaceX = VolumeBox.Min.X + iShiftX + (XandY % iPlacesForObjectsX) * ObjectPlaceSize;
		int32 iPlaceY = VolumeBox.Min.Y + iShiftY + (XandY / iPlacesForObjectsX) * ObjectPlaceSize;
		int32 iPlaceZ = VolumeBox.Min.Z + iShiftZ + (iRandPlaceNumber / (iPlacesForObjectsX * iPlacesForObjectsY)) * ObjectPlaceSize;

		int32 iRnd = rand() % MaterialsNum;
		SpawnCrumb(FVector(iPlaceX, iPlaceY, iPlaceZ), iRnd);
	}
}

AActor* ALevelSettings::SpawnOtherObserver()
{
	// Spawn!
	AStaticMeshActor *pGameObject = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator(0, 0, 0));
	pGameObject->SetMobility(EComponentMobility::Movable);
	TArray<UStaticMeshComponent*> Comps;
	pGameObject->GetComponents(Comps);
	if (Comps.Num() > 0)
	{
		UStaticMeshComponent* FoundComp = Comps[0];
		FoundComp->SetStaticMesh(m_observerMesh);

		FoundComp->SetMaterial(0, m_observerMaterial);
		FoundComp->SetCastShadow(true);
		FoundComp->SetGenerateOverlapEvents(true);
		FCollisionResponseContainer collision_response;
		collision_response.SetAllChannels(ECollisionResponse::ECR_Overlap);
		FoundComp->SetCollisionResponseToChannels(collision_response);
		//FoundComp->OnComponentBeginOverlap.AddUniqueDynamic(ALevelSettings::GetInstance(), &ALevelSettings::OnGameObjectOverlapBegin);
		/*{ // add sound component
			if (CrumbMusic)
			{
				UAudioComponent* NewAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CrumbMusic, pGameObject->GetActorLocation());
				pGameObject->AddInstanceComponent(NewAudioComponent);
				NewAudioComponent->Play();
			}
		}*/
	}

	/*TSet<AActor*> aOverlapActors;
	pGameObject->UpdateOverlaps();
	pGameObject->GetOverlappingActors(aOverlapActors);
	if (aOverlapActors.Num())
	{
		pGameObject->Destroy();
		return nullptr;
	}*/
	return pGameObject;
}

AActor* ALevelSettings::SpawnCrumb(FVector pos, int32 crumbMaterialNum)
{
	int32 MaterialsNum = GameObjectMaterials.Num();
	check(crumbMaterialNum < MaterialsNum);
	if (crumbMaterialNum >= MaterialsNum)
	{
		return nullptr;
	}
	// Spawn!
	AStaticMeshActor *pGameObject = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), pos, FRotator(0, 0, 0));
	pGameObject->SetMobility(EComponentMobility::Static);
	TArray<UStaticMeshComponent*> Comps;
	pGameObject->GetComponents(Comps);
	pGameObject->SetActorScale3D({ 0.5f, 0.5f, 0.5f });
	if (Comps.Num() > 0)
	{
		UStaticMeshComponent* FoundComp = Comps[0];
		FoundComp->SetStaticMesh(SphereMesh);

		FoundComp->SetMaterial(0, GameObjectMaterials[crumbMaterialNum]);
		FoundComp->SetCastShadow(false);
		FoundComp->SetGenerateOverlapEvents(true);
		FCollisionResponseContainer collision_response;
		collision_response.SetAllChannels(ECollisionResponse::ECR_Overlap);
		FoundComp->SetCollisionResponseToChannels(collision_response);
		//FoundComp->OnComponentBeginOverlap.AddUniqueDynamic(ALevelSettings::GetInstance(), &ALevelSettings::OnGameObjectOverlapBegin);
		{ // add sound component
			if (CrumbMusic)
			{
				UAudioComponent* NewAudioComponent = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), CrumbMusic, pGameObject->GetActorLocation());
				pGameObject->AddInstanceComponent(NewAudioComponent);
				NewAudioComponent->Play();
			}
		}
	}

	TSet<AActor*> aOverlapActors;
	pGameObject->UpdateOverlaps();
	pGameObject->GetOverlappingActors(aOverlapActors);
	if (aOverlapActors.Num())
	{
		pGameObject->Destroy();
		return nullptr;
	}
	return pGameObject;
}

