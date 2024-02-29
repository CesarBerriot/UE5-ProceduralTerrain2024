#include "TerrainGenerator.h"

#include "ProceduralMeshComponent.h"

ATerrainGenerator::ATerrainGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("Proceduralmesh");
	ProceduralMeshComponent->SetupAttachment(RootComponent);
}

void ATerrainGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	FVector TopLeft = GetActorLocation();
	FVector TopRight = TopLeft + FVector::RightVector * GridPadding * GridWidth;
	FVector BottomLeft = TopLeft + FVector::ForwardVector * GridPadding * GridWidth;
	FVector BottomRight = BottomLeft + FVector::RightVector * GridPadding * GridWidth;
	UWorld * World = GetWorld();
	DrawDebugLine(World, TopLeft, TopRight, FColor::Green);
	DrawDebugLine(World, TopRight, BottomRight, FColor::Green);
	DrawDebugLine(World, BottomRight, BottomLeft, FColor::Green);
	DrawDebugLine(World, BottomLeft, TopLeft, FColor::Green);
}

void ATerrainGenerator::Generate()
{
	Clear();
	FVector Location = GetActorLocation();
	UWorld * World = GetWorld();
	for(int i = 0; i < GridWidth; ++i)
	{
		TArray<AActor*> Line = {};
		for(int i2 = 0; i2 < GridWidth; ++i2)
		{
			AActor * Cube = World->SpawnActor<AActor>();
			check(Cube)
			Line.Add(Cube);
			UStaticMeshComponent * Mesh = NewObject<UStaticMeshComponent>(Cube);
			check(Mesh)
			UMaterialInstanceDynamic * Material = UMaterialInstanceDynamic::Create(CubeMaterial, Mesh);
			Mesh->SetMaterial(0, Material);
			Mesh->SetStaticMesh(CubeMesh);
			Mesh->RegisterComponent();
			Cube->SetRootComponent(Mesh);
			Cube->RegisterAllComponents();
			Cube->SetActorLocation(
				Location +
				FVector::RightVector * i * GridPadding +
				FVector::ForwardVector * i2 * GridPadding
			);
		}
		Cubes.Add(Line);
	}
	if(UseChunks)
		GenerateWhiteNoiseInChunks();
	else
		GenerateWhiteNoise();
}

void ATerrainGenerator::Clear()
{
	for(auto Line : Cubes)
		for(auto Cube : Line)
			if(Cube)
				Cube->Destroy();
	Cubes.Empty();
}

void ATerrainGenerator::Blur()
{
	// Cache original grid
	TArray<TArray<float>> OriginalGrid = {};
	for(TArray Line : Cubes)
	{
		TArray<float> LineLocations = {};
		for(AActor * Cube : Line)
			LineLocations.Add(Cube->GetActorLocation().Z);
		OriginalGrid.Add(LineLocations);
	}

	// Gaussian blur 1x
	for(int i = 0; i < GridWidth; ++i)
		for(int i2 = 0; i2 < GridWidth; ++i2)
		{
			int iInc = i < (GridWidth - 1) ? i + 1 : i;
			int iDec = i ? i - 1 : 0;
			int i2Inc = i2 < (GridWidth - 1) ? i2 + 1 : i2;
			int i2Dec = i2 ? i2 - 1 : 0;
			float Current = OriginalGrid[i][i2];
			float Left = OriginalGrid[iDec][i2];
			float Right = OriginalGrid[iInc][i2];
			float Up = OriginalGrid[i][i2Dec];
			float Down = OriginalGrid[i][i2Inc];
			FVector Location = Cubes[i][i2]->GetActorLocation();
			float NewZ = (Current + Left + Right + Up + Down) / 5;
			Cubes[i][i2]->SetActorLocation(FVector(Location.X, Location.Y, FMath::Lerp(Current, NewZ, .5)));
		}

	ApplyCubeColors();
}

void ATerrainGenerator::ApplyToProceduralMesh()
{
	ProceduralMeshComponent->ClearMeshSection(0);
	FVector Location = GetActorLocation();
	TArray<FVector> Vertices = {};
	TArray<int> Triangles = {};
	for(auto Line : Cubes)
		for(auto Cube : Line)
			Vertices.Add(Cube->GetActorLocation() - Location);
	for(int i = 0; i < GridWidth - 1; ++i)
		for(int i2 = 0; i2 < GridWidth - 1; ++i2)
		{
			int Current = i * GridWidth + i2;
			int Right = Current + 1;
			int Down = Current + GridWidth;
			int DownRight = Right + GridWidth;
			Triangles.Append({ DownRight, Right, Current });
			Triangles.Append({ Down, DownRight, Current });
		}
	ProceduralMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Triangles, {}, {}, {}, {}, false);
}

void ATerrainGenerator::GenerateWhiteNoise()
{
	for(auto Line : Cubes)
		for(auto Cube : Line)
			Cube->SetActorLocation(
				Cube->GetActorLocation() +
				FVector::UpVector * FMath::RandRange(MinHeight, MaxHeight)
			);
	ApplyCubeColors();
}

void ATerrainGenerator::GenerateWhiteNoiseInChunks()
{
	checkf(!(GridWidth % ChunksWidth), L"T'es con")
	int Max = GridWidth / ChunksWidth;
	for(int i = 0; i < Max; ++i)
		for(int i2 = 0; i2 < Max; ++i2)
		{
			float NewHeight = GetActorLocation().Z +
			                  FMath::RandRange(MinHeight, MaxHeight);
			for(int i3 = 0; i3 < ChunksWidth; ++i3)
				for(int i4 = 0; i4 < ChunksWidth; ++i4)
				{
					AActor * Cube = Cubes[i * ChunksWidth + i3][i2 * ChunksWidth + i4];
					FVector Location = Cube->GetActorLocation();
					Cube->SetActorLocation(FVector(Location.X, Location.Y, NewHeight));
				}
		}
	ApplyCubeColors();
}

void ATerrainGenerator::ApplyCubeColors()
{
	for(TArray Line : Cubes)
		for(auto Cube : Line)
		{
			float CubeHeight = Cube->GetActorLocation().Z - GetActorLocation().Z - MinHeight;
			float MaxHeightFromZero = MaxHeight - MinHeight;
			UStaticMeshComponent * Mesh = (UStaticMeshComponent*)Cube->GetRootComponent();
			UMaterialInstanceDynamic * Material = (UMaterialInstanceDynamic*)Mesh->GetMaterial(0);
			Material->SetVectorParameterValue(
				"Color",
				FMath::Lerp(
					FLinearColor::Green,
					FLinearColor::Red,
					CubeHeight / MaxHeightFromZero
				)
			);
		}
}
