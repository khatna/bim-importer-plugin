// Fill out your copyright notice in the Description page of Project Settings.


#include "BIMScene.h"

#include "DXFRuntimeImporter.h"
#include "HttpModule.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/DefaultLogger.hpp"
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
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->OnProcessRequestComplete().BindUObject(SceneObj, &UBIMScene::OnBIMDownloaded);
	Req->SetVerb(TEXT("GET"));
	Req->SetURL(Path);
	Req->ProcessRequest();
	
	return SceneObj;
}

void UBIMScene::OnBIMDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	
	if (!bWasSuccessful || Response->GetResponseCode() / 100 == 4 || Response->GetResponseCode() / 100 == 5) // better way?
	{
		UE_LOG(LogAssimp, Warning, TEXT("Request unsuccessful"))
		return;
	}
	
	UE_LOG(LogAssimp, Warning, TEXT("Request Successful"))
	const FString ResContent = Response->GetContentAsString();
	const char* Content = TCHAR_TO_ANSI(*ResContent);
	const unsigned Length = ResContent.Len();

	GEngine->AddOnScreenDebugMessage(1, 10.0f, FColor::White, FString::FromInt(Length));
	
	/*
	 * Import scene using Assimp, with postprocessing flags
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
	
	const aiScene* Scene = aiImportFileFromMemory(Content, Length, Flags, "");
	
	if (!Scene)
	{
		UE_LOG(LogAssimp, Error, TEXT("BIM failed to import."))
		GEngine->AddOnScreenDebugMessage(2, 10.0f, FColor::Red, TEXT("There was an error while importing the BIM"));
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
