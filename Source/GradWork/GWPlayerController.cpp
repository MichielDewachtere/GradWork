// Fill out your copyright notice in the Description page of Project Settings.


#include "GWPlayerController.h"

#include "EnhancedInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "PerformanceLogger.h"

void AGWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	int currentScene = m_CurrentScene;
	if (currentScene == -1)
		currentScene = 0;
	
	m_pPerformanceLogger = new FPerformanceLogger(30.f, 0.1f, m_SceneNames[currentScene].ToString(), GetPlayerModeString());

	if (m_bIsSimulating && m_CurrentScene != 4)
	{
		m_CurrentPos = 0;
		GetPawn()->SetActorTransform(m_Positions[m_CurrentPos]);
	}
}

void AGWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		subsystem->AddMappingContext(m_pInputMappingContext, 0);
	
	if (UEnhancedInputComponent* enhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		enhancedInputComponent->BindAction(m_pInputToSwitchScenes, ETriggerEvent::Triggered, this, &AGWPlayerController::SwitchScene);
		enhancedInputComponent->BindAction(m_pInputToSwitchPos, ETriggerEvent::Triggered, this, &AGWPlayerController::SwitchPos);	
		enhancedInputComponent->BindAction(m_pInputToTakeScreenshot, ETriggerEvent::Triggered, this, &AGWPlayerController::TakeScreenshot);	
		enhancedInputComponent->BindAction(m_pInputToTrackStats, ETriggerEvent::Triggered, this, &AGWPlayerController::TrackStats);	
		enhancedInputComponent->BindAction(m_pInputToStartSimulation, ETriggerEvent::Triggered, this, &AGWPlayerController::StartSimulation);	
	}
}

void AGWPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_pPerformanceLogger->Update(DeltaTime);

	if (m_bIsSimulating == false)
		return;

	constexpr int offset = 20;
	if (m_AccuTime > offset && m_AccuTime < offset + 0.5)
		m_pPerformanceLogger->StartTracking();
	
	m_AccuTime += DeltaTime;

	if (m_AccuTime > m_SimulationTimePerScene)
	{
		TakeScreenshot_Helper(m_CurrentScene, m_CurrentPos);
		
		++m_CurrentScene;
		if (m_CurrentScene >= m_SceneNames.Num())
		{
			m_CurrentScene = 0;
			m_bIsSimulating = false;
		}

		UGameplayStatics::OpenLevel(GetWorld(), m_SceneNames[m_CurrentScene]);
	}
}


void AGWPlayerController::SwitchScene(const FInputActionValue& value)
{
	if (m_bIsSimulating || m_pPerformanceLogger->IsTracking())
		return;
	
	m_CurrentScene = GetWrappedIndex(m_CurrentScene, value.Get<bool>() ? 1 : -1, m_SceneNames.Num());
	UGameplayStatics::OpenLevel(GetWorld(), m_SceneNames[m_CurrentScene]);
}

void AGWPlayerController::SwitchPos(const FInputActionValue& value)
{
	if (m_pPerformanceLogger->IsTracking())
		return;
	
	m_CurrentPos = GetWrappedIndex(m_CurrentPos, value.Get<bool>() ? 1 : -1, m_Positions.Num());
	GetPawn()->SetActorTransform(m_Positions[m_CurrentPos]);
}

void AGWPlayerController::TakeScreenshot(const FInputActionValue& value)
{
	auto curPos = m_CurrentPos;
	if (m_CurrentPos == -1)
		curPos = 0;

	auto curScene = m_CurrentScene;
	if (m_CurrentScene == -1)
		curScene = 0;
	
	TakeScreenshot_Helper(curScene, curPos);
}

void AGWPlayerController::TrackStats(const FInputActionValue&)
{
	if (m_bIsSimulating)
		return;
	
	if (m_CurrentPos != 0 && m_CurrentScene != 4)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "Not at correct position to track stats!");
		return;
	}
	
	m_pPerformanceLogger->StartTracking();
}

void AGWPlayerController::StartSimulation(const FInputActionValue&)
{
	if (m_bIsSimulating == false)
	{
		m_bIsSimulating = true;
		// m_pPerformanceLogger->StopTracking();
		m_pPerformanceLogger->StartTracking();
		
		m_CurrentScene = 0;
		UGameplayStatics::OpenLevel(GetWorld(), m_SceneNames[m_CurrentScene]);
	}
}

FString AGWPlayerController::GetPlayerModeString() const
{
	switch (m_CurrentMode)
	{
	case EMode::odt:
		return "Order-Dependant Transparency";
	case EMode::oit:
		return "Order-Independent Transparency";
	case EMode::raytracing:
		return "Raytracing";
	}
	
	return "";
}

void AGWPlayerController::TakeScreenshot_Helper(const int curScene, const int curPos) const
{
	FString fileName = m_SceneNames[curScene].ToString() + '_';
	fileName.AppendInt(curPos);
	
	const FString timestamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d"));
	const FString directoryPath = FPaths::ProjectDir() / "Screenshots" / timestamp;
	if (!FPaths::DirectoryExists(directoryPath))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*directoryPath);
	}

	const FString fullFilePath = directoryPath / fileName;
	FScreenshotRequest::RequestScreenshot(fullFilePath, false, false);

	UE_LOG(LogTemp, Log, TEXT("Screenshot saved to: %s"), *fullFilePath);
}

int32 AGWPlayerController::GetWrappedIndex(const int32 currentIndex, const int32 increment, const int32 max)
{
	return (currentIndex + increment + max) % max;
}