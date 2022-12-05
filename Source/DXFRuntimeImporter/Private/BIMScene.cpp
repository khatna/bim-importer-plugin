// Fill out your copyright notice in the Description page of Project Settings.


#include "BIMScene.h"

#include "DXFRuntimeImporter.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/DefaultLogger.hpp"

/*
 * the world object. I.e. destroying the UBIMScene might not destroy the mesh and line actors.
 */
UBIMScene* UBIMScene::ImportScene(const FString Path, float RefEasting, float RefNorthing, float RefAltitude, UObject* Outer)
{
	Assimp::DefaultLogger::set(new UEAssimpStream());
	
	// Create new scene object
	UBIMScene* SceneObj = NewObject<UBIMScene>(Outer, StaticClass());

	// Import scene using Assimp, with postprocessing flags
	
	/*
	 * available Flags: https://assimp.sourceforge.net/lib_html/postprocess_8h.html 
	 */
	constexpr unsigned Flags = (
		aiProcess_CalcTangentSpace          |
		aiProcess_GenNormals                |
		aiProcess_JoinIdenticalVertices	    |
		aiProcess_RemoveRedundantMaterials  |
		aiProcess_SplitLargeMeshes          |
		aiProcess_Triangulate               |
		aiProcess_GenUVCoords               |
		aiProcess_SortByPType               |
		aiProcess_FindDegenerates           |
		aiProcess_FindInvalidData
	);
	
	const aiScene* BaseScene = aiImportFile(TCHAR_TO_UTF8(*Path), Flags);

	// if the base scene wasn't imported for some reason, return empty obj.
	if (!BaseScene) return SceneObj;
	
	SceneObj->BaseScene = BaseScene;
	SceneObj->RefEasting = RefEasting;
	SceneObj->RefNorthing = RefNorthing;
	SceneObj->RefAltitude = RefAltitude;

	// Set meshes and lines
	for (int i = 0; i < BaseScene->mNumMeshes; i++)
	{
		aiMesh* Obj = BaseScene->mMeshes[i];
		if (Obj->mPrimitiveTypes & aiPrimitiveType_LINE)
		{
			SceneObj->LineObjs.Add(Obj);
		}
		else if (Obj->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) // assume triangulated (via flags)
		{
			SceneObj->MeshObjs.Add(Obj);
		}
		// TODO: if point?
	}
	
	return SceneObj;
}

void UBIMScene::SpawnMeshes(UMaterialInstance* MeshMaterial)
{
	// Spawn meshes
	for (int i = 0; i < MeshObjs.Num(); i++)
	{
		aiMesh* AiMesh = MeshObjs[i];
		UE_LOG(LogAssimp, Log, TEXT("Spawning: %s"), UTF8_TO_TCHAR(AiMesh->mName.C_Str()))
		
		// Spawn an actor
		ABIMMeshActor* MeshActor = GetWorld()->SpawnActor<ABIMMeshActor>(
			FVector(0.0, 0.0,0.0),
			FRotator(0.0,0.0,0.0));

		// Add to actor array in scene
		MeshActor->SetRefs(RefEasting, RefNorthing, RefAltitude);
		MeshActor->SetMaterial(MeshMaterial);
		MeshActors.Add(MeshActor);

		// Generate the RMC in spawned actor
		MeshActor->GenerateMesh(AiMesh);
	}
}

void UBIMScene::SpawnLines(UMaterialInstance* LineMaterial)
{
	// Spawn lines (similar to meshes)
	for (int i = 0; i < LineObjs.Num(); i++)
	{
		aiMesh* AiMesh = LineObjs[i];
		UE_LOG(LogAssimp, Log, TEXT("Spawning: %s"), UTF8_TO_TCHAR(AiMesh->mName.C_Str()))

		ABIMPolyLineActor* LineActor = GetWorld()->SpawnActor<ABIMPolyLineActor>(
			FVector(0.0f, 0.0f, 0.0f),
			FRotator(0.0f, 0.0f, 0.0f)
		);

		LineActor->SetRefs(RefEasting, RefNorthing, RefAltitude);
		LineActor->SetMaterial(LineMaterial);
		LineActors.Add(LineActor);

		LineActor->GenerateMesh(AiMesh);
	}
}

TArray<ABIMMeshActor*> UBIMScene::GetAllMeshActors()
{
	return MeshActors;
}

TArray<ABIMPolyLineActor*> UBIMScene::GetAllPolyLines()
{
	return LineActors;
}

void UBIMScene::BeginDestroy()
{
	// release assimp resources
	aiReleaseImport(BaseScene);

	// null assimp pointers
	BaseScene = nullptr;
	MeshObjs.Empty();
	LineObjs.Empty();

	// destroy "child" actors
	for (ABIMMeshActor* Actor : MeshActors)
	{
		Actor->Destroy();
	}

	for (ABIMPolyLineActor* Actor : LineActors)
	{
		Actor->Destroy();
	}
	
	UObject::BeginDestroy();
}
