// Fill out your copyright notice in the Description page of Project Settings.


#include "PPSettings.h"
#include "../LevelSettings.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "array"
#include "ParallelPhysics.h"

static UPPSettings *s_PPSettings = nullptr;

UPPSettings::UPPSettings()
{
}

void UPPSettings::Init(UWorld *World)
{
	s_PPSettings = this;
	check(World);

	if (bUseCppUniverseSize)
	{
		UniverseBox = ALevelSettings::GetInstance()->GetUniverseBoundingBox();
	}
}

UPPSettings* UPPSettings::GetInstance()
{
	return s_PPSettings;
}

void UPPSettings::ConvertGeometry(UWorld *World)
{
	check(World);
	FVector UniverseSize = UniverseBox.GetSize();
	PPh::VectorInt32Math pphUniverseSize(UniverseSize.X / UniverseEtherCellSize, UniverseSize.Y / UniverseEtherCellSize, UniverseSize.Z / UniverseEtherCellSize);
	int8 ThreadsCountTmp = ThreadsCount;
	if (bCalculateNearObserver)
	{
		ThreadsCountTmp = 0;
	}
	bool bParallelPhysicsInit = PPh::ParallelPhysics::Init(pphUniverseSize, ThreadsCountTmp);
	check(bParallelPhysicsInit);

	for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		FBox ActorBox = ActorItr->GetComponentsBoundingBox();
		FVector ActorBoxSize = ActorBox.GetSize();
		for (int32 xx = UniverseEtherCellSize / 2; xx <= ActorBoxSize.X; xx += UniverseEtherCellSize)
		{
			for (int32 yy = UniverseEtherCellSize / 2; yy <= ActorBoxSize.Y; yy += UniverseEtherCellSize)
			{
				for (int32 zz = UniverseEtherCellSize / 2; zz <= ActorBoxSize.Z; zz += UniverseEtherCellSize)
				{
					if (FMath::PointBoxIntersection(FVector(ActorBox.Min.X + xx, ActorBox.Min.Y + yy, ActorBox.Min.Z + zz), UniverseBox))
					{
						TArray<UStaticMeshComponent*> Comps;
						ActorItr->GetComponents(Comps);
						if (Comps.Num() > 0)
						{
							check(1 == Comps.Num()); // Support one UStaticMeshComponent for now
							// count universe coords
							FVector Location(ActorBox.Min.X + xx, ActorBox.Min.Y + yy, ActorBox.Min.Z + zz);
							PPh::VectorInt32Math universePos = ConvertLocationToPPhPosition(Location);
							/*universePos.m_posX = (ActorBox.Min.X + xx - UniverseBox.Min.X) / UniverseEtherCellSize;
							check(0 <= universePos.m_posX && universePos.m_posX < pphUniverseSize.m_posX);
							universePos.m_posY = (ActorBox.Min.Y + yy - UniverseBox.Min.Y) / UniverseEtherCellSize;
							check(0 <= universePos.m_posY && universePos.m_posY < pphUniverseSize.m_posY);
							universePos.m_posZ = (ActorBox.Min.Z + zz - UniverseBox.Min.Z) / UniverseEtherCellSize;
							check(0 <= universePos.m_posZ && universePos.m_posZ < pphUniverseSize.m_posZ);*/

							const TArray<class UMaterialInstance *>& GameObjectMaterials = ALevelSettings::GetInstance()->GetGameObjectMaterials();
							bool isCrumb = false;
							for (int ii = 0; ii < GameObjectMaterials.Num(); ++ii)
							{
								if (GameObjectMaterials[ii] == Comps[0]->GetMaterial(0))
								{
									static std::array<FColor, 4> Colors = { FColor::Green, FColor::Yellow, FColor::Red, FColor::Blue };
									if (ii < Colors.size())
									{
										PPh::EtherColor etherColor(Colors[ii].B, Colors[ii].G, Colors[ii].R);
										bool bInitEtherCell = PPh::ParallelPhysics::GetInstance()->InitEtherCell(universePos, PPh::EtherType::Crumb, etherColor);
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
								PPh::EtherColor etherColor(blockGrayColor, blockGrayColor, blockGrayColor);
								bool bInitEtherCell = PPh::ParallelPhysics::GetInstance()->InitEtherCell(universePos, PPh::EtherType::Block, etherColor);
								check(bInitEtherCell);
							}
						}
					}
				}
			}
		}
	}
}

int32 RoundToMinMaxPPhInt(double value)
{
	int64 val64 = (int64)(0.5 + value);
	if (0 > val64)
	{
		val64 = std::max(val64, (int64)PPh::OrientationVectorMath::PPH_INT_MIN);
	}
	else
	{
		val64 = std::min(val64, (int64)PPh::OrientationVectorMath::PPH_INT_MAX);
	}
	return (int32)val64;
}

int32 FixFloatErrors(int32 component, int32 maxComponentValue)
{
	int32 componentCorrect = component;
	if (std::abs(component) == maxComponentValue)
	{
		if (0 > component)
		{
			componentCorrect = PPh::OrientationVectorMath::PPH_INT_MIN;
		}
		else
		{
			componentCorrect = PPh::OrientationVectorMath::PPH_INT_MAX;
		}
	}
	return componentCorrect;
}

PPh::VectorInt32Math UPPSettings::ConvertLocationToPPhPosition(const FVector &Location)
{
	FVector UniverseBoxMin = UPPSettings::GetInstance()->UniverseBox.Min;
	int32 UniverseEtherCellSize = UPPSettings::GetInstance()->UniverseEtherCellSize;
	PPh::VectorInt32Math universePos;
	PPh::VectorInt32Math pphUniverseSize = PPh::ParallelPhysics::GetInstance()->GetUniverseSize();
	universePos.m_posX = (Location.X - UniverseBoxMin.X) / UniverseEtherCellSize;
	check(0 <= universePos.m_posX && universePos.m_posX < pphUniverseSize.m_posX);
	universePos.m_posX = std::max(0, universePos.m_posX);
	universePos.m_posX = std::min(pphUniverseSize.m_posX - 1, universePos.m_posX);

	universePos.m_posY = (Location.Y - UniverseBoxMin.Y) / UniverseEtherCellSize;
	check(0 <= universePos.m_posY && universePos.m_posY < pphUniverseSize.m_posY);
	universePos.m_posY = std::max(0, universePos.m_posY);
	universePos.m_posY = std::min(pphUniverseSize.m_posY - 1, universePos.m_posY);

	universePos.m_posZ = (Location.Z - UniverseBoxMin.Z) / UniverseEtherCellSize;
	check(0 <= universePos.m_posZ && universePos.m_posZ < pphUniverseSize.m_posZ);
	universePos.m_posZ = std::max(0, universePos.m_posZ);
	universePos.m_posZ = std::min(pphUniverseSize.m_posZ - 1, universePos.m_posZ);

	return universePos;
}

PPh::OrientationVectorMath UPPSettings::ConvertRotationToPPhOrientation(const FRotator &Rotator)
{
	FVector orientationVector = Rotator.Vector();
	return ConvertRotationToPPhOrientation(orientationVector);
}

PPh::OrientationVectorMath UPPSettings::ConvertRotationToPPhOrientation(const FVector &orientationVector)
{
	float maxComponent = std::max(std::max(std::abs(orientationVector.X), std::abs(orientationVector.Y)), std::abs(orientationVector.Z));
	double factor = 0;
	if (maxComponent > 0)
	{
		factor = PPh::OrientationVectorMath::PPH_INT_MAX / (double)maxComponent;
	}

	PPh::OrientationVectorMath pphOrientation(RoundToMinMaxPPhInt(orientationVector.X*factor), RoundToMinMaxPPhInt(orientationVector.Y*factor),
		RoundToMinMaxPPhInt(orientationVector.Z*factor));

	int32 maxPPhComponent = std::max(std::max(std::abs(pphOrientation.m_posX), std::abs(pphOrientation.m_posY)), std::abs(pphOrientation.m_posZ));
	pphOrientation.m_posX = FixFloatErrors(pphOrientation.m_posX, maxPPhComponent);
	pphOrientation.m_posY = FixFloatErrors(pphOrientation.m_posY, maxPPhComponent);
	pphOrientation.m_posZ = FixFloatErrors(pphOrientation.m_posZ, maxPPhComponent);

	return pphOrientation;
}
