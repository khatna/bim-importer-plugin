// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BIMMeshActor.h"
#include "BIMPolyLineActor.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "assimp/scene.h"
#include "BIMScene.generated.h"

/**
 * The UAIScene class imports a DXF scene using the assimp
 * library, and builds the meshes contained in the scene.
 * It exposes references to UAIMesh objects.
 *
 * TODO: review potential memory leak in BaseScene, MeshObjs and LineObjs
 */
UCLASS(Blueprintable,BlueprintType)
class DXFRUNTIMEIMPORTER_API UBIMScene : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Import scene (assimp supported file) from given path
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer")
	static UBIMScene* ImportScene(FString Path, float RefEasting, float RefNorthing, float RefAltitude, UObject* Outer);

	/**
	 * Build and spawn triangle meshes contained in this scene
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer|Scene")
	void SpawnMeshes(UMaterialInstance* MeshMaterial);
	
	/**
	 * Return an array containing pointers to every mesh in this scene
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer|Scene")
	TArray<ABIMMeshActor*> GetAllMeshActors();

	/**
	* Return an array containing pointers to every polyline mesh in this scene
	*/
	UFUNCTION(BlueprintCallable, Category="DXF Importer|Scene")
	TArray<ABIMPolyLineActor*> GetAllPolyLines();

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float RefEasting;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float RefNorthing;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float RefAltitude;
	
private:
	const aiScene* BaseScene;

	// underlying assimp triangle mesh
	TArray<aiMesh*> MeshObjs;

	// underlying assimp line mesh objects
	TArray<aiMesh*> LineObjs;
	
	UPROPERTY(Transient)
	TArray<ABIMMeshActor*> MeshActors;

	UPROPERTY(Transient)
	TArray<ABIMPolyLineActor*> LineActors;
};
