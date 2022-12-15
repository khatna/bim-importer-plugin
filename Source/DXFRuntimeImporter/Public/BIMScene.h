// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BIMMeshActor.h"
#include "BIMPolyLineActor.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "assimp/scene.h"
#include "HttpModule.h"
#include "BIMScene.generated.h"

/**
 * The UAIScene class imports a DXF (or other supported format) scene using the assimp
 * library. A file path, UTM reference coordinates and an outer object reference are required
 */
UCLASS(Blueprintable,BlueprintType)
class DXFRUNTIMEIMPORTER_API UBIMScene : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Import scene (assimp supported file) from given path and automatically render it.
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer")
	static UBIMScene* ImportScene(FString BIMUrl, float RefEasting, float RefNorthing, float RefAltitude, UMaterialInstance* MeshMaterial, UMaterialInstance* LineMaterial, UObject* Outer);

	/**
	 * Build and spawn triangle meshes contained in this scene
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer|Scene")
	void SpawnMeshes();
	
	/**
	 * Build and spawn point and line drawings contained in this scene
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer|Scene")
	void SpawnLines();
	
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

	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	UMaterialInstance* MeshMaterial;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite)
	UMaterialInstance* LineMaterial;
	
	// The outer object that parents the scene (useful for GC and spawning)
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	UObject* Outer;
	
protected:
	virtual void BeginDestroy() override;
	
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

	// HTTP Callback when BIM model is received
	void OnBIMDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
