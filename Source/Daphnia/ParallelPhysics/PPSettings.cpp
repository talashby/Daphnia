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
		FVector vecOrigin, vecBoxExtent;
		ALevelSettings::GetInstance()->GetUniversityBounds(vecOrigin, vecBoxExtent);
		UniverseBox = FBox::BuildAABB(vecOrigin, vecBoxExtent);
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
		FVector vecOrigin, vecBoxExtent;
		ActorItr->GetActorBounds(false, vecOrigin, vecBoxExtent);
		FBox ActorBox = FBox::BuildAABB(vecOrigin, vecBoxExtent);
		for (int32 xx = UniverseEtherCellSize / 2; xx < vecBoxExtent.X * 2; xx += UniverseEtherCellSize)
		{
			for (int32 yy = UniverseEtherCellSize / 2; yy < vecBoxExtent.Y * 2; yy += UniverseEtherCellSize)
			{
				for (int32 zz = UniverseEtherCellSize / 2; zz < vecBoxExtent.Z * 2; zz += UniverseEtherCellSize)
				{
					if (FMath::PointBoxIntersection(FVector(xx,yy,zz), UniverseBox))
					{
						TArray<UStaticMeshComponent*> Comps;
						ActorItr->GetComponents(Comps);
						if (Comps.Num() > 0)
						{
							check(1 == Comps.Num()); // Support one UStaticMeshComponent for now
							// count universe coords
							int32 universePosX = ((vecOrigin.X - vecBoxExtent.X + xx) - UniverseBox.Min.X) / UniverseEtherCellSize;
							check(universePosX < ParallelPhysics::GetXDimension());
							int32 universePosY = ((vecOrigin.Y - vecBoxExtent.Y + yy) - UniverseBox.Min.Y) / UniverseEtherCellSize;
							check(universePosY < ParallelPhysics::GetYDimension());
							int32 universePosZ = ((vecOrigin.Z - vecBoxExtent.Z + zz) - UniverseBox.Min.Z) / UniverseEtherCellSize;
							check(universePosZ < ParallelPhysics::GetXDimension());

							const TArray<class UMaterialInstance *>& GameObjectMaterials = ALevelSettings::GetInstance()->GetGameObjectMaterials();
							for (int ii = 0; ii < GameObjectMaterials.Num(); ++ii)
							{
								if (GameObjectMaterials[ii] == Comps[0]->GetMaterial(0))
								{
									static std::array<FColor, 4> Colors = { FColor::Green, FColor::Yellow, FColor::Red, FColor::Blue };
									if (ii < Colors.size())
									{
										bool bInitEtherCell = ParallelPhysics::InitEtherCell(universePosX, universePosY, universePosZ, Colors[ii].R, Colors[ii].G, Colors[ii].B);
										check(bParallelPhysicsInit);
									}
									else
									{
										checkNoEntry();
									}
								}
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
