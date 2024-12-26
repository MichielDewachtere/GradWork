#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <algorithm>
#include <fstream>
#include <functional>

class FPerformanceLogger
{
public:
    explicit FPerformanceLogger(float inDurationSeconds, float outlierPercentage, const FString& fileName, const FString& folderName);
    
    void Update(float deltaTime);
    
    bool IsTracking() const { return m_bIsTracking; }
    void StartTracking();
    void StopTracking();
    
private:
    struct FStatEntry
    {
        double frameTime;
        double gameThreadTime;
        double renderThreadTime;
        double gpuTime;
        int32 drawCalls;
        double usedPhysicalMemoryMB;
        double usedVirtualMemoryMB;
    };

    FString m_FileName, m_FolderName;
    float m_DurationSeconds;
    float m_OutlierPercentage;
    float m_ElapsedTime;
    bool m_bIsTracking;
    std::vector<FStatEntry> m_StatsData;

    static int32 TrackDrawCalls();
    void ProcessAndSaveStats();
    
    template<typename T>
    std::vector<T> ExtractMetric(std::function<T(const FStatEntry&)> metricGetter);
    template<typename T>
    void LogStats(const FString& statName, const FString& filepath, const std::vector<T>& data);

    FString GetLogFilePath() const;
    static void EnsureDirectoryExists(const FString& directoryPath);
    template <typename T>
    static FString FormatStatsRow(const FString& StatName, T Min, T Max, double Average);
    template <typename T>
    void CalculateStats(const std::vector<T>& data, T& outMin, T& outMax, double& outAverage) const;
};

template <typename T>
std::vector<T> FPerformanceLogger::ExtractMetric(std::function<T(const FStatEntry&)> metricGetter)
{
    std::vector<T> metricValues;
    for (const auto& entry : m_StatsData)
    {
        metricValues.push_back(metricGetter(entry));
    }
    return metricValues;
}

template <typename T>
void FPerformanceLogger::LogStats(const FString& statName, const FString& filepath, const std::vector<T>& data)
{
    T min, max;
    double average;

    // Calculate stats
    CalculateStats(data, min, max, average);

    // Ensure the directory exists
    EnsureDirectoryExists(FPaths::GetPath(filepath));

    // Write stats to the file
    if (std::ofstream file(TCHAR_TO_UTF8(*filepath), std::ios::app); file.is_open())
    {
        // If the file is empty, write the header
        file.seekp(0, std::ios::end);
        if (file.tellp() == 0)
        {
            file << "Stat Name              | Min       | Max       | Average\n";
            file << "--------------------------------------------------------\n";
        }

        // Write formatted stats
        file << TCHAR_TO_UTF8(*FormatStatsRow(statName, min, max, average));
        file.close();
    }

    UE_LOG(LogTemp, Log, TEXT("Logged %s stats to: %s"), *statName, *filepath);
}

template <typename T>
FString FPerformanceLogger::FormatStatsRow(const FString& StatName, T Min, T Max, double Average)
{
    return FString::Printf(
        TEXT("%-22s | %-9.2f | %-9.2f | %-9.2f\n"),
        *StatName, static_cast<float>(Min), static_cast<float>(Max), static_cast<float>(Average)
    );
}

template <typename T>
void FPerformanceLogger::CalculateStats(const std::vector<T>& data, T& outMin, T& outMax, double& outAverage) const
{
    std::vector<T> sortedData = data;
    std::sort(sortedData.begin(), sortedData.end());

    size_t trimCount = static_cast<size_t>(sortedData.size() * m_OutlierPercentage);
    double sum = 0.0;
    size_t count = 0;

    for (size_t i = trimCount; i < sortedData.size() - trimCount; ++i)
    {
        sum += sortedData[i];
        ++count;
    }
    
    auto minMax = std::minmax_element(sortedData.begin(), sortedData.end());
    outMin = *minMax.first;
    outMax = *minMax.second;

    outAverage = (count > 0) ? (sum / count) : 0.0;
}
