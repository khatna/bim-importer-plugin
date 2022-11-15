// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "assimp/mesh.h"
#include "AIPolyLine.generated.h"

/**
 * A polyline class. Takes in an aiMesh representing a polyline
 * and builds a static mesh composed of cylinder strips.
 */
UCLASS()
class DXFRUNTIMEIMPORTER_API UAIPolyLine : public UObject
{
	GENERATED_BODY()
	
public:
	// The UE4 static mesh object built from Assimp scene data
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	UStaticMesh* StaticMesh;

	// Internal function to build a static mesh description from Assimp mesh structure
	UStaticMesh* BuildPolyLineMesh(aiMesh*);

private:
	// Pointer to Assimp base mesh struct
	aiMesh* BaseMesh = nullptr;
};
