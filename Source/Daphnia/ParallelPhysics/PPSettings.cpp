// Fill out your copyright notice in the Description page of Project Settings.


#include "PPSettings.h"
#include "../LevelSettings.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "array"
#include "ParallelPhysics.h"

UPPSettings::UPPSettings()
{
}

void UPPSettings::Init(UWorld *World)
{
	check(World);

	if (bUseCppUniverseSize)
	{
		UniverseBox = ALevelSettings::GetInstance()->GetUniverseBoundingBox();
	}
}

void UPPSettings::ConvertGeometry(UWorld *World)
{
	check(World);
	FVector UniverseSize = UniverseBox.GetSize();
	bool bParallelPhysicsInit = ParallelPhysics::Init(UniverseSize.X/UniverseEtherCellSize, UniverseSize.Y/UniverseEtherCellSize, UniverseSize.Z/UniverseEtherCellSize);
	check(bParallelPhysicsInit);

	for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		FBox ActorBox = ActorItr->GetComponentsBoundingBox();
		FVector ActorBoxSize = ActorBox.GetSize();
		for (int32 xx = UniverseEtherCellSize / 2; xx < ActorBoxSize.X; xx += UniverseEtherCellSize)
		{
			for (int32 yy = UniverseEtherCellSize / 2; yy < ActorBoxSize.Y; yy += UniverseEtherCellSize)
			{
				for (int32 zz = UniverseEtherCellSize / 2; zz < ActorBoxSize.Z; zz += UniverseEtherCellSize)
				{
					if (FMath::PointBoxIntersection(FVector(ActorBox.Min.X + xx, ActorBox.Min.Y + yy, ActorBox.Min.Z + zz), UniverseBox))
					{
						TArray<UStaticMeshComponent*> Comps;
						ActorItr->GetComponents(Comps);
						if (Comps.Num() > 0)
						{
							check(1 == Comps.Num()); // Support one UStaticMeshComponent for now
							// count universe coords
							int32 universePosX = (ActorBox.Min.X + xx - UniverseBox.Min.X) / UniverseEtherCellSize;
							check(0 <= universePosX && universePosX < ParallelPhysics::GetDimensionX());
							int32 universePosY = (ActorBox.Min.Y + yy - UniverseBox.Min.Y) / UniverseEtherCellSize;
							check(0 <= universePosY && universePosY < ParallelPhysics::GetDimensionY());
							int32 universePosZ = (ActorBox.Min.Z + zz - UniverseBox.Min.Z) / UniverseEtherCellSize;
							check(0 <= universePosZ && universePosZ < ParallelPhysics::GetDimensionX());

							const TArray<class UMaterialInstance *>& GameObjectMaterials = ALevelSettings::GetInstance()->GetGameObjectMaterials();
							bool isCrumb = false;
							for (int ii = 0; ii < GameObjectMaterials.Num(); ++ii)
							{
								if (GameObjectMaterials[ii] == Comps[0]->GetMaterial(0))
								{
									static std::array<FColor, 4> Colors = { FColor::Green, FColor::Yellow, FColor::Red, FColor::Blue };
									if (ii < Colors.size())
									{
										bool bInitEtherCell = ParallelPhysics::InitEtherCell(universePosX, universePosY, universePosZ,
											ParallelPhysics::EtherType::Crumb, Colors[ii].R, Colors[ii].G, Colors[ii].B);
										check(bInitEtherCell);
										isCrumb = true;
										break;
									}
									else
									{
										checkNoEntry();
									}
								}
							}
							if (!isCrumb)
							{
								constexpr uint8 blockGrayColor = 50;
								bool bInitEtherCell = ParallelPhysics::InitEtherCell(universePosX, universePosY, universePosZ,
									ParallelPhysics::EtherType::Block, blockGrayColor, blockGrayColor, blockGrayColor);
								check(bInitEtherCell);
							}
						}
					}
				}
			}
		}
		/*
		if (pGameObjectComponent)
		{
			for (int ii = 0; ii < GameObjectMaterials.Num(); ++ii)
			{
				if (GameObjectMaterials[ii] == OverlappedComp->GetMaterial(0))
				{
					// todo
				}
			}
		} */

		
		/*FString SceneActorName = ActorItr->GetName();
		if (!SceneActorName.Compare(sActorName) || sActorName.IsEmpty())
		{
			pOutActor = *ActorItr;
			return true;
		}*/
	}
}
