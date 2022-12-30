// Fill out your copyright notice in the Description page of Project Settings.


#include "BIMScene.h"

#include "DXFRuntimeImporter.h"
#include "HttpModule.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "Interfaces/IHttpResponse.h"

/*
 * the world object. I.e. destroying the UBIMScene might not destroy the mesh and line actors.
 */
UBIMScene* UBIMScene::ImportScene(const FString Path, float RefEasting, float RefNorthing, float RefAltitude, UMaterialInstance* MeshMaterial, UMaterialInstance* LineMaterial, UObject* Outer)
{
	Assimp::DefaultLogger::set(new UEAssimpStream());
	
	// Create new scene object and set params
	UBIMScene* SceneObj = NewObject<UBIMScene>(Outer, StaticClass());
	
	SceneObj->RefEasting = RefEasting;
	SceneObj->RefNorthing = RefNorthing;
	SceneObj->RefAltitude = RefAltitude;
	SceneObj->MeshMaterial = MeshMaterial;
	SceneObj->LineMaterial = LineMaterial;
	SceneObj->Outer = Outer;
	
	// Start processing request for BIM model
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(SceneObj, &UBIMScene::OnBIMDownloaded);
	Request->SetVerb(TEXT("GET"));
	Request->SetURL(Path);
	Request->ProcessRequest();

	// disable garbage collection while BIM model is downloading
	SceneObj->AddToRoot();
	
	return SceneObj;
}

void UBIMScene::OnBIMDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetContentLength() <= 0)
	{
		UE_LOG(LogAssimp, Warning, TEXT("Request unsuccessful"))
		return;	
	}
	
	/*
	 * Import scene using Assimp, with postprocessing flags
	 * available Flags: https://assimp.sourceforge.net/lib_html/postprocess_8h.html 
	 */
	
	constexpr unsigned Flags = (
		aiProcess_GenNormals                |
		aiProcess_RemoveRedundantMaterials  |
		aiProcess_SplitLargeMeshes          |
		aiProcess_Triangulate               |
		aiProcess_SortByPType               |
		aiProcess_FindDegenerates           |
		aiProcess_FindInvalidData           |
		aiProcess_OptimizeGraph             |
		aiProcess_OptimizeMeshes            |
		aiProcess_SplitLargeMeshes
	);
	
	// Read scene from HTTP response buffer and import bim model
	UE_LOG(LogAssimp, Log, TEXT("Import Begin"))
	Assimp::Importer *Imp = new Assimp::Importer(); 
	const void * Buffer = Response->GetContent().GetData();
	const size_t Length = Response->GetContentLength();
	// have to provide "glb" as hint to import GLTF files for some reason.
	// DXF files still work given "glb" as hint, since importer falls back to signature based detection
	const aiScene* Scene = Imp->ReadFileFromMemory(Buffer, Length, Flags, "glb");
	UE_LOG(LogAssimp, Log, TEXT("Import End"))
	
	if (!Scene)
	{
		UE_LOG(LogAssimp, Error, TEXT("BIM failed to import."))
		GEngine->AddOnScreenDebugMessage(2, 10.0f, FColor::Red, TEXT("There was an error while importing the BIM"));
		BaseScene = nullptr;
		RemoveFromRoot();
		return;
	}
	
	// Set meshes and lines	
	for (unsigned int i = 0; i < Scene->mNumMeshes; i++)
	{
		aiMesh* Obj = Scene->mMeshes[i];
		if (Obj->mPrimitiveTypes & aiPrimitiveType_LINE)
		{
			this->LineObjs.Add(Obj);
		}
		else if (Obj->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) // assume triangulated (via flags)
		{
			this->MeshObjs.Add(Obj);
		}
		// TODO: if point?
	}

	this->BaseScene = Scene;

	SpawnLines();
	SpawnMeshes();

	// Remove from root to re-enable garbage collection
	RemoveFromRoot();
}

void UBIMScene::SpawnMeshes()
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

void UBIMScene::SpawnLines()
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

void UBIMScene::HideScene()
{
	for (ABIMMeshActor* Actor : MeshActors)
	{
		Actor->SetActorHiddenInGame(true);
	}

	for (ABIMPolyLineActor* Actor : LineActors)
	{
		Actor->SetActorHiddenInGame(true);
	}
}

void UBIMScene::ShowScene()
{
	for (ABIMMeshActor* Actor : MeshActors)
	{
		Actor->SetActorHiddenInGame(false);
	}

	for (ABIMPolyLineActor* Actor : LineActors)
	{
		Actor->SetActorHiddenInGame(false);
	}
}

void UBIMScene::BeginDestroy()
{
	if (BaseScene)
	{
		// release assimp resources
		aiReleaseImport(BaseScene);
	}

	// null assimp pointers
	BaseScene = nullptr;
	MeshObjs.Empty();
	LineObjs.Empty();

	// hide actors
	HideScene();

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
