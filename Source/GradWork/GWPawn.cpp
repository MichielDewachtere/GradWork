// Fill out your copyright notice in the Description page of Project Settings.


#include "GWPawn.h"

#include "Camera/CameraComponent.h"


// Sets default values
AGWPawn::AGWPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGWPawn::BeginPlay()
{
	Super::BeginPlay();

	if (auto* pCameraComponent = GetComponentByClass<UCameraComponent>())
	{
		// Set the camera's post-process settings
		FPostProcessSettings postProcessSettings;
		postProcessSettings.bOverride_AutoExposureBias = true;
		postProcessSettings.AutoExposureBias = 0.0f; // Set to a neutral value

		// Apply settings to the camera
		pCameraComponent->PostProcessSettings = postProcessSettings;
	}
}

// Called every frame
void AGWPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AGWPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

