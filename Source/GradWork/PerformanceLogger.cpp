// Fill out your copyright notice in the Description page of Project Settings.


#include "PerformanceLogger.h"

FPerformanceLogger::FPerformanceLogger(float inDurationSeconds, float outlierPercentage, const FString& fileName, const FString& folderName)
	: m_FileName(fileName)
	, m_FolderName(folderName)
	, m_DurationSeconds(inDurationSeconds)
	, m_OutlierPercentage(outlierPercentage)
	, m_ElapsedTime(0.0f)
	, m_bIsTracking(false)
{
}

void FPerformanceLogger::Update(const float deltaTime)
{
	if (m_bIsTracking == false)
		return;
	
	m_ElapsedTime += deltaTime;

	// Capture stats
	const double currentTime = FApp::GetCurrentTime();
	const double frameTime = (currentTime - FApp::GetLastTime()) * 1000.0;
	const double gameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	const double renderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	const double gpuCycles = RHIGetGPUFrameCycles();
	const double gpuTime = FPlatformTime::ToMilliseconds(gpuCycles);
	const int32 drawCalls = TrackDrawCalls();

	// Capture memory usage
	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	const double usedPhysicalMemoryMB = MemoryStats.UsedPhysical / (1024.0 * 1024.0);
	const double usedVirtualMemoryMB = MemoryStats.UsedVirtual / (1024.0 * 1024.0);
	
	// Store stats
	m_StatsData.push_back({ frameTime, gameThreadTime, renderThreadTime, gpuTime, drawCalls, usedPhysicalMemoryMB, usedVirtualMemoryMB });

	// If duration is reached, stop tracking and process stats
	if (m_ElapsedTime >= m_DurationSeconds)
	{
		StopTracking();
	}

	const auto string = FString::Printf(TEXT("Elapsed Time: %f / %f"), m_ElapsedTime, m_DurationSeconds);
	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Yellow, string);
}

void FPerformanceLogger::StartTracking()
{
	if (!m_bIsTracking)
	{
		m_StatsData.clear();
		m_ElapsedTime = 0.0f;
		m_bIsTracking = true;
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Started tracking performance");
		UE_LOG(LogTemp, Log, TEXT("Performance tracking started."));
	}
}

void FPerformanceLogger::StopTracking()
{
	if (m_bIsTracking)
	{
		m_bIsTracking = false;
		ProcessAndSaveStats();
		m_StatsData.clear();
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Stopped tracking performance");
		UE_LOG(LogTemp, Log, TEXT("Performance tracking stopped."));
	}
}

int32 FPerformanceLogger::TrackDrawCalls()
{
	// Use a temporary variable to safely transfer data back to the game thread
	int32 drawCallsThisFrame = 0;

	// Ensure this code runs on the rendering thread
	ENQUEUE_RENDER_COMMAND(GetDrawCallsCommand)(
		[&drawCallsThisFrame](FRHICommandListImmediate&)
		{
			// Sum up draw calls across all contexts
			for (int32 i = 0; i < 8; i++)
			{
				drawCallsThisFrame += GNumDrawCallsRHI[i];
			}
		});

	// Use a render fence to wait for the render thread to finish
	FRenderCommandFence renderFence;
	renderFence.BeginFence();
	renderFence.Wait();

	return drawCallsThisFrame;
}

void FPerformanceLogger::ProcessAndSaveStats()
{
	if (m_StatsData.empty())
		return;

	const FString filePath = GetLogFilePath();
	
	LogStats("FrameTime - ms", filePath, ExtractMetric<double>([](const FStatEntry& entry) -> double { return entry.frameTime; }));
	LogStats("GameThreadTime - ms", filePath, ExtractMetric<double>([](const FStatEntry& entry) { return entry.gameThreadTime; }));
	LogStats("RenderThreadTime - ms", filePath, ExtractMetric<double>([](const FStatEntry& entry) { return entry.renderThreadTime; }));
	LogStats("GPUTime - ms", filePath, ExtractMetric<double>([](const FStatEntry& entry) { return entry.gpuTime; }));
	LogStats("DrawCalls", filePath, ExtractMetric<int32>([](const FStatEntry& entry) { return static_cast<double>(entry.drawCalls); }));
	LogStats("Physical Memory - MB", filePath, ExtractMetric<double>([](const FStatEntry& entry) { return entry.usedPhysicalMemoryMB; }));
	LogStats("Virtual Memory - MB", filePath, ExtractMetric<double>([](const FStatEntry& entry) { return entry.usedVirtualMemoryMB; }));

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, "Written stats to " + filePath);
}

FString FPerformanceLogger::GetLogFilePath() const
{
	const FDateTime now = FDateTime::Now();
	const FString dateString = now.ToString(TEXT("%Y-%m-%d"));
	const FString modeString = m_FolderName;
	const FString directory = FPaths::ProjectDir() / "PerformanceLogs" / dateString / modeString;
	const FString extension = ".txt";
   
	FString uniqueFilePath = directory / m_FileName + extension;
	int32 suffix = 1;
   
	while (FPaths::FileExists(uniqueFilePath))
	{
		uniqueFilePath = FPaths::Combine(directory, FString::Printf(TEXT("%s_%d%s"), *m_FileName, suffix, *extension));
		++suffix;
	}
   
	return uniqueFilePath;
}

void FPerformanceLogger::EnsureDirectoryExists(const FString& directoryPath)
{
	if (!FPaths::DirectoryExists(directoryPath))
	{
		FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*directoryPath);
	}
}
