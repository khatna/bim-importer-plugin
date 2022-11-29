// Fill out your copyright notice in the Description page of Project Settings.


#include "BIMScene.h"

#include "DXFRuntimeImporter.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/DefaultLogger.hpp"

UBIMScene* UBIMScene::ImportScene(const FString Path, float RefEasting, float RefNorthing, float RefAltitude, UObject* Outer)
{
	Assimp::DefaultLogger::set(new UEAssimpStream());
	
	// Create new scene object
	UBIMScene* SceneObj = NewObject<UBIMScene>(Outer, StaticClass());

	// Import scene using Assimp and initialize it
	// available Flags: https://assimp.sourceforge.net/lib_html/postprocess_8h.html
	constexpr unsigned Flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FindInvalidData;
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
		if (Obj->mPrimitiveTypes & aiPrimitiveType_LINE || Obj->mPrimitiveTypes & aiPrimitiveType_POINT)
		{
			SceneObj->LineObjs.Add(Obj);
		}
		else // TRIANGLE OR POLYGON MESH
		{
			SceneObj->MeshObjs.Add(Obj);
		}
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
		MeshActor->Rename(UTF8_TO_TCHAR(AiMesh->mName.C_Str()));
		MeshActors.Add(MeshActor);

		// Generate the RMC in spawned actor
		MeshActor->GenerateMesh(AiMesh);
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



