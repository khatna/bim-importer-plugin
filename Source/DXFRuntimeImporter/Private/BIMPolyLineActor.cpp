// Fill out your copyright notice in the Description page of Project Settings.


#include "BIMPolyLineActor.h"

#include "Providers/RuntimeMeshProviderStatic.h"

#define SECTOR_COUNT 10
#define RADIUS 2.0f

// Sets default values
ABIMPolyLineActor::ABIMPolyLineActor()
{
	StaticProvider = CreateDefaultSubobject<URuntimeMeshProviderStatic>(TEXT("Static Provider"));
	StaticProvider->SetShouldSerializeMeshData(false);
	PrimaryActorTick.bCanEverTick = false;
}

// ---------------------------------------------------------------------------------------------------------------------

// returns points on unit circle
// TODO: precalculate trig functions and test performance
FVector GetSectorPoint(int Index)
{
	const float Phi = Index * (2.0f * PI) / static_cast<float>(SECTOR_COUNT);
	return FVector(0.0f, -FMath::Cos(Phi), FMath::Sin(Phi));
}

// Store cylinder positions, coords, etc. to supplied arrays
void CreateCylinder(FVector &Top, FVector &Bot, TArray<FVector> &Positions, TArray<FVector> &Normals, TArray<int32> &Triangles)
{
	// First index
	int FirstIndex = Positions.Num();
	
	// calculate and set direction of vertices
	FVector Direction = Top - Bot;
	Direction.Normalize();
	const FQuat Q = Direction.Rotation().Quaternion();

	for (int i = 0; i < SECTOR_COUNT; i++)
	{
		FVector BotPoint = RADIUS * GetSectorPoint(i);
		BotPoint = Q * BotPoint + Bot;
		Positions.Push(BotPoint);
	}
	
	for (int i = 0; i < SECTOR_COUNT; i++)
	{
		FVector TopPoint = RADIUS * GetSectorPoint(i);
		TopPoint = Q * TopPoint + Top;
		Positions.Push(TopPoint);
	}

	Positions.Push(Bot);
	Positions.Push(Top);
	
	// Set triangles
	for (int i = 0; i < SECTOR_COUNT; i++)
	{
		// indices of created vertices
		int BotCur = FirstIndex + i;
		int BotNext = FirstIndex + (i + 1) % SECTOR_COUNT;
		int TopCur = BotCur + SECTOR_COUNT;
		int TopNext = BotNext + SECTOR_COUNT;
		int BotCenter = Positions.Num() - 2;
		int TopCenter = Positions.Num() - 1;

		// Side triangles
		Triangles.Append({ BotCur, BotNext, TopNext });
		Triangles.Append({ BotCur, TopNext, TopCur });

		// Cap triangles
		Triangles.Append({ BotNext, BotCur, BotCenter });
		Triangles.Append({ TopCur, TopNext, TopCenter });
	}
}

void ABIMPolyLineActor::GenerateMesh(aiMesh* AiMesh)
{
	if (BaseMesh || !AiMesh) return;
	
	// Initialized and Clear RMC
	GetRuntimeMeshComponent()->Initialize(StaticProvider);
	StaticProvider->ClearSection(0, 0);
	
	// Set RMC material
	StaticProvider->SetupMaterialSlot(0, TEXT("BIM Line Material"), Material);
	
	TArray<FVector> Positions;
	TArray<FVector> Normals;
	TArray<int32> Triangles;
	TArray<FRuntimeMeshTangent> Tangents;
	TArray<FVector2D> TexCoords;
	
	// For each line, create a cylinder
	for (int f = 0; f < AiMesh->mNumFaces; f++)
	{
		// Create and store cylinder in buffers
		const aiFace Face = AiMesh->mFaces[f];
		const aiVector3D BotVec = AiMesh->mVertices[Face.mIndices[0]];
		const aiVector3D TopVec = AiMesh->mVertices[Face.mIndices[1]];
		FVector Top = FVector(TopVec.y - RefNorthing, TopVec.x - RefEasting, TopVec.z - RefAltitude);
		FVector Bot = FVector(BotVec.y - RefNorthing, BotVec.x - RefEasting, BotVec.z - RefAltitude);
		CreateCylinder(Top, Bot, Positions, Normals, Triangles);
	}

	
	// Create RMC section from cylinder
	const TArray<FColor> EmptyColors{};
	StaticProvider->CreateSectionFromComponents(0, 0, 0, Positions, Triangles, Normals, TexCoords, EmptyColors, Tangents, ERuntimeMeshUpdateFrequency::Infrequent, false);
	
	// Set base mesh for future reference (maybe)	
	BaseMesh = AiMesh;
}

// ---------------------------------------------------------------------------------------------------------------------

void ABIMPolyLineActor::SetRefs(float Easting, float Northing, float Altitude)
{
	RefEasting = Easting;
	RefNorthing = Northing;
	RefAltitude = Altitude;
}

void ABIMPolyLineActor::SetMaterial(UMaterialInstance* MaterialInstance)
{
	// change material
	Material = MaterialInstance;

	if (GetRuntimeMeshComponent()->GetProvider())
	{
		StaticProvider->SetupMaterialSlot(0, TEXT("BIM Material"), Material);
	}
}
