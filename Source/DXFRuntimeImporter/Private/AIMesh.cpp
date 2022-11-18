// Fill out your copyright notice in the Description page of Project Settings.

#include "AIMesh.h"
#include "AIScene.h"
#include "DXFRuntimeImporter.h"
#include "StaticMeshDescription.h"

UStaticMesh* UAIMesh::BuildStaticMesh(aiMesh* Mesh)
{
	if (StaticMesh) return StaticMesh;

	// Set Parent scene
	const UAIScene* ParentScene = Cast<UAIScene>(GetOuter());
	if (ParentScene == nullptr)
	{
		UE_LOG(LogAssimp, Fatal, TEXT("Mesh not parented to an imported scene"))
		return nullptr;
	}
	
	UE_LOG(LogAssimp, Log, TEXT("Building Mesh: %s"), UTF8_TO_TCHAR(Mesh->mName.C_Str()))

	// In runtime: create a static mesh description, and built from it
	UStaticMeshDescription* MeshDesc = UStaticMesh::CreateStaticMeshDescription(this);
	
	// Add vertices, set positions and normals
	MeshDesc->ReserveNewVertices(Mesh->mNumVertices);
	MeshDesc->ReserveNewVertexInstances(Mesh->mNumVertices);
	for (int v = 0; v < Mesh->mNumVertices; v++)
	{
		FVertexID V = MeshDesc->CreateVertex();
		MeshDesc->CreateVertexInstance(V);
	}

	// Set positions, normals, and UVs
	TVertexInstanceAttributesRef<FVector> Normals = MeshDesc->GetVertexInstanceNormals();
	float N = FMath::Sqrt(Mesh->mNumVertices);
	for (int v = 0; v < Mesh->mNumVertices; v++)
	{
		aiVector3D Position = Mesh->mVertices[v];
		aiVector3D Normal = Mesh->mNormals[v];

		FVector UEPosition = FVector(
			Position.y - ParentScene->RefNorthing,
			Position.x - ParentScene->RefEasting,
			Position.z - ParentScene->RefAltitude
		);

		// Flip X and Y in Unreal Engine
		MeshDesc->SetVertexPosition(FVertexID(v), UEPosition);
		Normals[FVertexInstanceID(v)] = FVector(Normal.y, Normal.x, Normal.z);
	}

	// Add faces (triangles)
	const FPolygonGroupID PolygonGroupID = MeshDesc->CreatePolygonGroup();
	MeshDesc->ReserveNewTriangles(Mesh->mNumFaces);
	MeshDesc->ReserveNewPolygons(Mesh->mNumFaces);
	MeshDesc->SetPolygonGroupMaterialSlotName(PolygonGroupID, FName(TEXT("Mesh Material")));
	
	int TrianglesSkipped = 0;
	for (int f = 0; f < Mesh->mNumFaces; f++)
	{
		const aiFace Face = Mesh->mFaces[f];
		FVertexInstanceID V0 = FVertexInstanceID(Face.mIndices[0]);
		FVertexInstanceID V1 = FVertexInstanceID(Face.mIndices[1]);
		FVertexInstanceID V2 = FVertexInstanceID(Face.mIndices[2]);

		// Unreal crashes if a triangle uses less than three distinct vertices
		if (V0.GetValue() == V1.GetValue() || V0.GetValue() == V2.GetValue() || V1.GetValue() == V2.GetValue())
		{
			TrianglesSkipped++;
			continue;
		}
		
		TArray<FEdgeID> NewEdgeIDs;
		MeshDesc->CreateTriangle(PolygonGroupID, { V0, V1, V2 }, NewEdgeIDs);
	}

	UE_LOG(LogAssimp, Warning, TEXT("Triangles skipped: %d out of %d"), TrianglesSkipped, Mesh->mNumFaces)

	// Build
	StaticMesh = NewObject<UStaticMesh>(this, FName(UTF8_TO_TCHAR(Mesh->mName.C_Str())));
	StaticMesh->SetStaticMaterials({ParentScene->MeshMaterial});
	StaticMesh->BuildFromStaticMeshDescriptions({MeshDesc});
	//StaticMesh->AddMaterial(ParentScene->MeshMaterial);
	//StaticMesh->SetMaterial(0, ParentScene->MeshMaterial);
	
	// Set params and return
	BaseMesh = Mesh;
	return StaticMesh;
}
