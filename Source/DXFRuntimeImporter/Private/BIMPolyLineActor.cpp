// Fill out your copyright notice in the Description page of Project Settings.


#include "BIMPolyLineActor.h"

#include "Providers/RuntimeMeshProviderStatic.h"

#define SECTOR_COUNT 10
#define RADIUS 50.0f

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
void CreateCylinder(FVector &Top, FVector &Bot, TArray<FVector> &PositionsLOD0, TArray<FVector> &PositionsLOD1, TArray<FVector> &PositionsLOD2, TArray<FVector> &Normals, TArray<int32> &Triangles)
{
	// First index
	int FirstIndex = PositionsLOD0.Num();
	
	// calculate and set direction of vertices
	FVector Direction = Top - Bot;
	Direction.Normalize();
	const FQuat Q = Direction.Rotation().Quaternion();

	for (int i = 0; i < SECTOR_COUNT; i++)
	{
		FVector BotPoint0 = RADIUS * GetSectorPoint(i);
		FVector BotPoint1 = RADIUS * 2.0 * GetSectorPoint(i);
		FVector BotPoint2 = RADIUS * 3.0 * GetSectorPoint(i);
		BotPoint0 = Q * BotPoint0 + Bot;
		BotPoint1 = Q * BotPoint1 + Bot;
		BotPoint2 = Q * BotPoint2 + Bot;
		PositionsLOD0.Push(BotPoint0);
		PositionsLOD1.Push(BotPoint1);
		PositionsLOD2.Push(BotPoint2);
	}
	
	for (int i = 0; i < SECTOR_COUNT; i++)
	{
		FVector TopPoint0 = RADIUS * GetSectorPoint(i);
		FVector TopPoint1 = RADIUS * 2.0 * GetSectorPoint(i);
		FVector TopPoint2 = RADIUS * 3.0 * GetSectorPoint(i);
		TopPoint0 = Q * TopPoint0 + Top;
		TopPoint1 = Q * TopPoint1 + Top;
		TopPoint2 = Q * TopPoint2 + Top;
		PositionsLOD0.Push(TopPoint0);
		PositionsLOD1.Push(TopPoint1);
		PositionsLOD2.Push(TopPoint2);
	}

	PositionsLOD0.Push(Bot);
	PositionsLOD0.Push(Top);
	PositionsLOD1.Push(Bot);
	PositionsLOD1.Push(Top);
	PositionsLOD2.Push(Bot);
	PositionsLOD2.Push(Top);
	
	// Set triangles
	for (int i = 0; i < SECTOR_COUNT; i++)
	{
		// indices of created vertices
		int BotCur = FirstIndex + i;
		int BotNext = FirstIndex + (i + 1) % SECTOR_COUNT;
		int TopCur = BotCur + SECTOR_COUNT;
		int TopNext = BotNext + SECTOR_COUNT;
		int BotCenter = PositionsLOD0.Num() - 2;
		int TopCenter = PositionsLOD0.Num() - 1;

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

	// Setup 3 LODs
	FRuntimeMeshLODProperties LOD0 = FRuntimeMeshLODProperties();
	LOD0.ScreenSize = 1.3f;
	FRuntimeMeshLODProperties LOD1 = FRuntimeMeshLODProperties();
	LOD1.ScreenSize = 0.8f;
	FRuntimeMeshLODProperties LOD2 = FRuntimeMeshLODProperties();
	LOD2.ScreenSize = 0.3f;
	StaticProvider->ConfigureLODs({LOD0, LOD1, LOD2});
	
	// Set RMC material
	StaticProvider->SetupMaterialSlot(0, TEXT("BIM Line Material"), Material);
	
	TArray<FVector> PositionsLOD0;
	TArray<FVector> PositionsLOD1;
	TArray<FVector> PositionsLOD2;
	TArray<FVector> Normals;
	TArray<int32> Triangles;
	TArray<FRuntimeMeshTangent> Tangents;
	TArray<FVector2D> TexCoords;
	
	// For each line, create a cylinder
	for (unsigned int f = 0; f < AiMesh->mNumFaces; f++)
	{
		// Create and store cylinder in buffers
		const aiFace Face = AiMesh->mFaces[f];
		const aiVector3D BotVec = AiMesh->mVertices[Face.mIndices[0]];
		const aiVector3D TopVec = AiMesh->mVertices[Face.mIndices[1]];
		// centimeter scaling
		FVector Top = 100.0f * FVector(TopVec.y - RefNorthing, TopVec.x - RefEasting, TopVec.z - RefAltitude);
		FVector Bot = 100.0f * FVector(BotVec.y - RefNorthing, BotVec.x - RefEasting, BotVec.z - RefAltitude);
		CreateCylinder(Top, Bot, PositionsLOD0, PositionsLOD1, PositionsLOD2, Normals, Triangles);
	}
	
	// Create RMC section from cylinder
	const TArray<FColor> EmptyColors{};

	StaticProvider->CreateSectionFromComponents(0, 0, 0, PositionsLOD0, Triangles, Normals, TexCoords, EmptyColors, Tangents, ERuntimeMeshUpdateFrequency::Infrequent, false);
	StaticProvider->CreateSectionFromComponents(1, 0, 0, PositionsLOD1, Triangles, Normals, TexCoords, EmptyColors, Tangents, ERuntimeMeshUpdateFrequency::Infrequent, false);
	StaticProvider->CreateSectionFromComponents(2, 0, 0, PositionsLOD2, Triangles, Normals, TexCoords, EmptyColors, Tangents, ERuntimeMeshUpdateFrequency::Infrequent, false);
	
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
