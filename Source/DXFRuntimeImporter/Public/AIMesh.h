// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "assimp/mesh.h"
#include "CoreUObject/Public/UObject/Object.h"
#include "AIMesh.generated.h"

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class DXFRUNTIMEIMPORTER_API UAIMesh : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int NumMeshes;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	UStaticMesh* StaticMesh;

	// Internal function to build a static mesh description from Assimp mesh structure
	UStaticMesh* BuildStaticMesh(aiMesh*, float RefEasting, float RefNorthing, float RefAltitude);

private:
	aiMesh* BaseMesh = nullptr;
};
