// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RuntimeMeshActor.h"
#include "assimp/mesh.h"
#include "BIMMeshActor.generated.h"

// TODO: investigate potential memory leak in BaseMesh
UCLASS()
class DXFRUNTIMEIMPORTER_API ABIMMeshActor : public ARuntimeMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABIMMeshActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInterface* Material;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RefEasting;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RefNorthing;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RefAltitude;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
	URuntimeMeshProviderStatic* StaticProvider;

public:
	void GenerateMesh(aiMesh* AiMesh);

	UFUNCTION()
	void SetRefs(float Easting, float Northing, float Altitude);

	UFUNCTION(BlueprintCallable)
	void SetMaterial(UMaterialInstance* MaterialInstance);
	
	UPROPERTY(Transient)
	TArray<FVector> Positions;
	UPROPERTY(Transient)
	TArray<int32> Triangles;
	UPROPERTY(Transient)
	TArray<FVector> Normals;
	UPROPERTY(Transient)
	TArray<FRuntimeMeshTangent> Tangents;
	UPROPERTY(Transient)
	TArray<FVector2D> TexCoords;
	
private:
	// Base mesh
	aiMesh* BaseMesh = nullptr;
};
