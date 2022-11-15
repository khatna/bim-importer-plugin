// Fill out your copyright notice in the Description page of Project Settings.


#include "AIScene.h"
#include "DXFRuntimeImporter.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/DefaultLogger.hpp"

UAIScene* UAIScene::ImportScene(const FString Path, float RefEasting, float RefNorthing, float RefAltitude)
{
	Assimp::DefaultLogger::set(new UEAssimpStream());
	
	// Create new scene object
	UAIScene* SceneObj = NewObject<UAIScene>(GetTransientPackage(), StaticClass());

	// Import scene using Assimp and initialize it
	constexpr unsigned Flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FindInvalidData;
	const aiScene* BaseScene = aiImportFile(TCHAR_TO_UTF8(*Path), Flags);
	
	SceneObj->BaseScene = BaseScene;
	SceneObj->RefEasting = RefEasting;
	SceneObj->RefNorthing = RefNorthing;
	SceneObj->RefAltitude = RefAltitude;
	
	return SceneObj;
}

// Only build meshes with normals (for testing)
void UAIScene::BuildScene()
{
	for (int i = 0; i < BaseScene->mNumMeshes; i++)
	{
		aiMesh* Mesh = BaseScene->mMeshes[i];
		if (!Mesh->HasNormals()) // lines, poly-lines etc. don't have normals
		{
			UAIPolyLine* LineObj = NewObject<UAIPolyLine>(this, UAIPolyLine::StaticClass());
			LineObj->BuildPolyLineMesh(Mesh);
			Lines.Add(LineObj);
		}
		else
		{
			// Create mesh object and set params. Set object parent to the imported scene
			UAIMesh* MeshObj = NewObject<UAIMesh>(this, UAIMesh::StaticClass());
			MeshObj->BuildStaticMesh(Mesh);
			Meshes.Add(MeshObj);
		}
	}
}

TArray<UAIMesh*> UAIScene::GetAllMeshes()
{
	return Meshes;
}



