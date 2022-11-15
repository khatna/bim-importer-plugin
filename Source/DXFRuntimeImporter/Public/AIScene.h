// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIMesh.h"
#include "AIPolyLine.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "assimp/scene.h"
#include "AIScene.generated.h"

/**
 * The UAIScene class imports a DXF scene using the assimp
 * library, and builds the meshes contained in the scene.
 * It exposes references to UAIMesh objects.
 *
 * TODO: review potential memory leak in BaseScene
 */
UCLASS(Blueprintable,BlueprintType)
class DXFRUNTIMEIMPORTER_API UAIScene : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Import scene (assimp supported file) from given path
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer")
	static UAIScene* ImportScene(FString Path, float RefEasting, float RefNorthing, float RefAltitude);

	/**
	 * Builds meshes contained in this scene
	 */
	UFUNCTION(BlueprintCallable, Category="DXF Importer|Scene")
	void BuildScene();
	
	/**
	 * Return an array containing pointers to every mesh in this scene
	 */
	UFUNCTION(BlueprintCallable)
	TArray<UAIMesh*> GetAllMeshes();

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float RefEasting;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float RefNorthing;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float RefAltitude;
	
private:
	const aiScene* BaseScene;
	
	UPROPERTY(Transient)
	TArray<UAIMesh*> Meshes;

	UPROPERTY(Transient)
	TArray<UAIPolyLine*> Lines;
};
