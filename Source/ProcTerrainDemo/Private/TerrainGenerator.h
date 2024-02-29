// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainGenerator.generated.h"

UCLASS()
class PROCTERRAINDEMO_API ATerrainGenerator : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere) float GridPadding = 150;
	UPROPERTY(EditAnywhere) int GridWidth = 20;
	UPROPERTY(EditAnywhere) int ChunksWidth = 4;
	UPROPERTY(EditAnywhere) UStaticMesh * CubeMesh;
	UPROPERTY(EditAnywhere) UMaterial * CubeMaterial;
	UPROPERTY(EditAnywhere) float MinHeight = 0;
	UPROPERTY(EditAnywhere) float MaxHeight = 1000;
	UPROPERTY(EditAnywhere) bool UseChunks = true;
	UPROPERTY(VisibleAnywhere) class UProceduralMeshComponent * ProceduralMeshComponent;
	TArray<TArray<AActor*>> Cubes;
public:
	ATerrainGenerator();
private:
	virtual void Tick(float DeltaSeconds) override;
	FORCEINLINE virtual bool ShouldTickIfViewportsOnly() const override { return true; }
	UFUNCTION(CallInEditor) void Generate();
	UFUNCTION(CallInEditor) void Clear();
	UFUNCTION(CallInEditor) void Blur();
	UFUNCTION(CallInEditor) void ApplyToProceduralMesh();
	void GenerateWhiteNoise();
	void GenerateWhiteNoiseInChunks();
	void ApplyCubeColors();
};
