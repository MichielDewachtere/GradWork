#include "TransparentHeavyLevel.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"

ATransparentHeavyLevel::ATransparentHeavyLevel()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // In the Level Script Actor
    m_pCubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    m_pBaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_Translucent_Emissive.M_Translucent_Emissive"));
}

void ATransparentHeavyLevel::BeginPlay()
{
    ALevelScriptActor::BeginPlay();

    SpawnCubes();
}

void ATransparentHeavyLevel::SpawnCubes() 
{
    // Ensure you have a reference to the cube mesh and material
    if (!m_pCubeMesh || !m_pBaseMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("CubeMesh or BaseMaterial not assigned!"));
        return;
    }

    // Player's position and view settings
    const auto playerLocation = FVector(0.f, 0.f, 0.f);
    constexpr float fov = 90.f; // Horizontal FOV
    constexpr float aspectRatio = 16.f / 9.f; // Screen aspect ratio

    // Calculate the frustum bounds
    constexpr float halfHorizontalFov = FMath::DegreesToRadians(fov / 2);
    const float halfVerticalFov = FMath::Atan(FMath::Tan(halfHorizontalFov) / aspectRatio);
    
    // Create a random stream with a specific seed for consistent randomness
    FRandomStream randomStream;
    randomStream.Initialize(1);

    constexpr int32 amountOfCubes{ 2500 };
    for (int32 i = 0; i < amountOfCubes; i++)
    {
        // Random distance within the bounds
        constexpr float maxDistance = 7500.f;
        constexpr float minDistance = 1000.f;
        // Non-linear distance calculation (quadratic distribution)
        const float randomFraction = randomStream.FRand();
        const float distance = FMath::Lerp(minDistance, maxDistance, FMath::Sqrt(randomFraction)); // Square root distribution

        // Random angle within the FOV
        const float horizontalAngle = static_cast<float>(randomStream.FRandRange(-halfHorizontalFov, halfHorizontalFov));
        const float verticalAngle = static_cast<float>(randomStream.FRandRange(-halfVerticalFov, halfVerticalFov));

        // Calculate the cube's position
        FVector direction = FVector(
            FMath::Cos(verticalAngle) * FMath::Cos(horizontalAngle),
            FMath::Cos(verticalAngle) * FMath::Sin(horizontalAngle),
            FMath::Sin(verticalAngle)
        ).GetSafeNormal();

        const FVector location = playerLocation + direction * distance;

        // Spawn the cube at the calculated location
        if (const auto* newCube = GetWorld()->SpawnActor<AStaticMeshActor>(location, FRotator::ZeroRotator))
        {
            // Set the mesh and material for the spawned cube
            newCube->GetStaticMeshComponent()->SetStaticMesh(m_pCubeMesh);

            // Create a dynamic material instance for random colors
            auto* dynMaterial = UMaterialInstanceDynamic::Create(m_pBaseMaterial, this);
            FLinearColor randomColor = FLinearColor::MakeRandomColor();
            randomColor *= 1.5f; // Brighten the color
            dynMaterial->SetVectorParameterValue(FName("Color"), randomColor);

            // Apply the material to the cube
            newCube->GetStaticMeshComponent()->SetMaterial(0, dynMaterial);
        }
    }
}
