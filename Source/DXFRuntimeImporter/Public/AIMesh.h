// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "assimp/mesh.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "AIMesh.generated.h"

/**
 * TODO: review potential memory leak in BaseMesh
 */
UCLASS(Blueprintable,BlueprintType)
class DXFRUNTIMEIMPORTER_API UAIMesh : public UObject
{
	GENERATED_BODY()

public:
	// The UE4 static mesh object built from Assimp scene data
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	UStaticMesh* StaticMesh;

	// Internal function to build a static mesh description from Assimp mesh structure
	UStaticMesh* BuildStaticMesh(aiMesh*);

private:
	// Pointer to Assimp base mesh struct
	aiMesh* BaseMesh = nullptr;
};