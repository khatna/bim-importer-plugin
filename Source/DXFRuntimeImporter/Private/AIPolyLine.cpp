// Fill out your copyright notice in the Description page of Project Settings.


#include "AIPolyLine.h"

#include "AIScene.h"
#include "DXFRuntimeImporter.h"
#include "StaticMeshDescription.h"

// Cylinder strip properties
#define SECTION_COUNT 3
#define RADIUS 2.0f

// returns points on +X facing unit circle. This represents the vertices of the cylinder
FVector GetSectionPoint(int SectionCount, int Index)
{
    float Phi = Index * (2.0f * PI) / static_cast<float>(SectionCount);
    return FVector(0.0f, -FMath::Cos(Phi), FMath::Sin(Phi));
}

// Create a cylinder section description
void CreateCylinder(FVector Bot, FVector Top, float Radius, UStaticMeshDescription* MeshDesc)
{
    // center + num_sections vertices on top and bottom of the cylinder
    MeshDesc->ReserveNewVertices(2 * (1 + SECTION_COUNT));
    MeshDesc->ReserveNewVertexInstances(2 * (1 + SECTION_COUNT));
    // 2 triangles on each rectangular side of the cylinder
    // 1 triangle per section on top and bottom
    MeshDesc->ReserveNewTriangles(4 * SECTION_COUNT);

    // add section vertices
    // save id of first vertex in cylinder for later use
    int VertexStart = -1;
    for (int i = 0; i < 2 * SECTION_COUNT; i++)
    {
        FVertexID V = MeshDesc->CreateVertex();
        MeshDesc->CreateVertexInstance(V);
        if (VertexStart == -1) VertexStart = V.GetValue();
    }
    
    // add center vertices
    FVertexID BotId = MeshDesc->CreateVertex();
    FVertexID TopId = MeshDesc->CreateVertex();
    FVertexInstanceID BotInstanceId = MeshDesc->CreateVertexInstance(BotId);
    FVertexInstanceID TopInstanceId = MeshDesc->CreateVertexInstance(TopId);  
    
    // add triangles
    FPolygonGroupID PolygonGroupID = MeshDesc->CreatePolygonGroup();
    
    for (int i = 0; i < SECTION_COUNT; i++)
    {
        FVertexInstanceID V0 = FVertexInstanceID(VertexStart + i);
        FVertexInstanceID V1 = FVertexInstanceID((i+1) % SECTION_COUNT + VertexStart);
        FVertexInstanceID V2 = FVertexInstanceID(V0.GetValue() + SECTION_COUNT);
        FVertexInstanceID V3 = FVertexInstanceID(V1.GetValue() + SECTION_COUNT);

        TArray<FEdgeID> NewEdgeIDs;

        // add triangles for sides - order of vertices determine normal dir
        MeshDesc->CreateTriangle(PolygonGroupID, { V0, V3, V1 }, NewEdgeIDs);
        MeshDesc->CreateTriangle(PolygonGroupID, { V0, V2, V3 }, NewEdgeIDs);

        // add triangles for the two caps - order of vertices determine normal dir
        MeshDesc->CreateTriangle(PolygonGroupID, {V1, V0, BotInstanceId}, NewEdgeIDs);
        MeshDesc->CreateTriangle(PolygonGroupID, {V2, V3, TopInstanceId}, NewEdgeIDs);
    }

    // calculate and set positions of vertices
    FVector Direction = Top - Bot;
    Direction.Normalize();
    FQuat Quaternion = Direction.Rotation().Quaternion();
    for (int i = 0; i < SECTION_COUNT; i++)
    {
        FVector BotPoint = Radius * GetSectionPoint(SECTION_COUNT, i);
        FVector TopPoint = Radius * GetSectionPoint(SECTION_COUNT, i);
        BotPoint = Quaternion * BotPoint + Bot;
        TopPoint = Quaternion * TopPoint + Top;
        MeshDesc->SetVertexPosition(FVertexID(VertexStart + i), BotPoint);
        MeshDesc->SetVertexPosition(FVertexID(VertexStart + i + SECTION_COUNT), TopPoint);
    }
    MeshDesc->SetVertexPosition(BotId, Bot);
    MeshDesc->SetVertexPosition(TopId, Top);
}

UStaticMesh* UAIPolyLine::BuildPolyLineMesh(aiMesh* Mesh)
{
    if (StaticMesh) return StaticMesh;
    
    // Set Parent scene
    const UAIScene* ParentScene = Cast<UAIScene>(GetOuter());
    if (ParentScene == nullptr)
    {
        UE_LOG(LogAssimp, Fatal, TEXT("Mesh not parented to an imported scene"))
        return nullptr;
    }
    
    UE_LOG(LogAssimp, Log, TEXT("Building line: %s"), UTF8_TO_TCHAR(Mesh->mName.C_Str()))

    UStaticMeshDescription* MeshDesc = UStaticMesh::CreateStaticMeshDescription(this);
    
    for (int f = 0; f < Mesh->mNumFaces; f++)
    {
        const aiFace Face = Mesh->mFaces[f];
        const aiVector3D BotVec = Mesh->mVertices[Face.mIndices[0]];
        const aiVector3D TopVec = Mesh->mVertices[Face.mIndices[1]];
        const FVector Top = FVector(
            TopVec.y - ParentScene->RefNorthing,
            TopVec.x - ParentScene->RefEasting,
            TopVec.z - ParentScene->RefAltitude
            );
        const FVector Bot = FVector(
            BotVec.y - ParentScene->RefNorthing,
            BotVec.x - ParentScene->RefEasting,
            BotVec.z - ParentScene->RefAltitude
        );
        CreateCylinder(Bot, Top, RADIUS, MeshDesc);
    }
    
    StaticMesh = NewObject<UStaticMesh>(this, FName(UTF8_TO_TCHAR(Mesh->mName.C_Str())));
    StaticMesh->SetMaterial(0, ParentScene->LineMaterial);
    StaticMesh->BuildFromStaticMeshDescriptions({MeshDesc});
    
    BaseMesh = Mesh;
    return StaticMesh;
}
