// Fill out your copyright notice in the Description page of Project Settings.


#include "BIMMeshActor.h"
#include "Providers/RuntimeMeshProviderStatic.h"
#include "DXFRuntimeImporter.h"

// Sets default values
ABIMMeshActor::ABIMMeshActor()
{
	StaticProvider = CreateDefaultSubobject<URuntimeMeshProviderStatic>(TEXT("Static Provider"));
	StaticProvider->SetShouldSerializeMeshData(false);
	PrimaryActorTick.bCanEverTick = false;
}

void ABIMMeshActor::GenerateMesh(aiMesh* AiMesh)
{
	// if null argument or base mesh already defined
	if (!AiMesh || BaseMesh) return;

	// Clear RMC
	GetRuntimeMeshComponent()->Initialize(StaticProvider);
	StaticProvider->ClearSection(0, 0);

	// reserve buffers
	Positions.Empty();
	Normals.Empty();
	Tangents.Empty();
	TexCoords.Empty();
	Positions.AddUninitialized(AiMesh->mNumVertices);
	Normals.AddUninitialized(AiMesh->mNumVertices);
	Tangents.AddUninitialized(AiMesh->mNumVertices);
	TexCoords.AddUninitialized(AiMesh->mNumVertices);
	
	// set positions, normals, tangents, UV
	for (int v = 0; v < AiMesh->mNumVertices; v++)
	{
		if (AiMesh->HasPositions())
		{
			Positions[v] = FVector(
				AiMesh->mVertices[v].y - RefNorthing,
				AiMesh->mVertices[v].x - RefEasting,
				AiMesh->mVertices[v].z - RefAltitude
			);	
		}

		if (AiMesh->HasNormals())
		{
			Normals[v] = FVector(
				AiMesh->mNormals[v].y,
				AiMesh->mNormals[v].x,
				AiMesh->mNormals[v].z
			);	
		}

		if (AiMesh->HasTangentsAndBitangents())
		{
			Tangents[v] = FVector(
				AiMesh->mTangents[v].y,
				AiMesh->mTangents[v].x,
				AiMesh->mTangents[v].z
			);
		}

		if (AiMesh->mTextureCoords[0])
		{
			// set UVs
		}
	}

	// set triangles
	int TrianglesSkipped = 0;
	for (int f = 0; f < AiMesh->mNumFaces; f++)
	{
		const aiFace Face = AiMesh->mFaces[f];

		// Unreal crashes if a triangle uses less than three distinct vertices
		if (
			Face.mIndices[0] == Face.mIndices[1] ||
			Face.mIndices[0] == Face.mIndices[2] ||
			Face.mIndices[1] == Face.mIndices[2]
			)
		{
			TrianglesSkipped++;
			continue;
		}

		Triangles.Push(Face.mIndices[0]);
		Triangles.Push(Face.mIndices[1]);
		Triangles.Push(Face.mIndices[2]);
	}
	
	UE_LOG(LogAssimp, Warning, TEXT("Triangles skipped: %d out of %d"), TrianglesSkipped, AiMesh->mNumFaces)

	// Create RMC section
	const TArray<FColor> EmptyColors{};
	StaticProvider->CreateSectionFromComponents(0, 0, 0, Positions, Triangles, Normals, TexCoords, EmptyColors, Tangents, ERuntimeMeshUpdateFrequency::Infrequent, false);
	StaticProvider->SetupMaterialSlot(0, TEXT("BIM Material"), Material);
	
	// Set base mesh for future reference (maybe)	
	BaseMesh = AiMesh;
}

void ABIMMeshActor::SetRefs(float Easting, float Northing, float Altitude)
{
	RefEasting = Easting;
	RefNorthing = Northing;
	RefAltitude = Altitude;
}

void ABIMMeshActor::SetMaterial(UMaterialInstance* MaterialInstance)
{
	// change material
	Material = MaterialInstance;

	if (GetRuntimeMeshComponent()->GetProvider())
	{
		StaticProvider->SetupMaterialSlot(0, TEXT("BIM Material"), Material);
	}
}
