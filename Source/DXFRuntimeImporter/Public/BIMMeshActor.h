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
	
private:
	// Base mesh
	aiMesh* BaseMesh = nullptr;
};
