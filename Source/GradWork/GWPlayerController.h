// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GWPlayerController.generated.h"

class FPerformanceLogger;
struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)  // This makes the enum available in both C++ and Blueprints
enum class EMode : uint8
{
	odt			UMETA(DisplayName = "Order-Dependant Transparency"),
	oit			UMETA(DisplayName = "Order-Independant Transparency"),
	raytracing	UMETA(DisplayName = "Raytracing"),
};

UCLASS()
class GRADWORK_API AGWPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	virtual void Tick(float DeltaTime) override;

private:
	TObjectPtr<FPerformanceLogger> m_pPerformanceLogger{ nullptr };
	
	UPROPERTY(EditAnywhere, Category = Input, DisplayName= "Input Mapping Context")
	UInputMappingContext* m_pInputMappingContext{ nullptr }; 
	UPROPERTY(EditAnywhere, Category = Input, DisplayName= "Input Action to Switch Scenes")
	UInputAction* m_pInputToSwitchScenes{ nullptr };
	UPROPERTY(EditAnywhere, Category = Input, DisplayName= "Input Action to Switch Positions")
	UInputAction* m_pInputToSwitchPos{ nullptr };
	UPROPERTY(EditAnywhere, Category = Input, DisplayName= "Input Action to Take Screenshot")
	UInputAction* m_pInputToTakeScreenshot{ nullptr };
	UPROPERTY(EditAnywhere, Category = Input, DisplayName= "Input Action to Track Stats")
	UInputAction* m_pInputToTrackStats{ nullptr };
	UPROPERTY(EditAnywhere, Category = Input, DisplayName= "Input Action to Start Simulation")
	UInputAction* m_pInputToStartSimulation{ nullptr };

	static inline bool m_bIsSimulating{ false };
	float m_AccuTime{ 0 }, m_SimulationTimePerScene{ 60 };
	
	UPROPERTY(EditAnywhere, DisplayName="Current Mode")
	EMode m_CurrentMode = EMode::odt;
	
	static inline int m_CurrentScene{ -1 };
	UPROPERTY(EditAnywhere, DisplayName="Scene Names")
	TArray<FName> m_SceneNames;

	int m_CurrentPos{ -1 };
	UPROPERTY(EditAnywhere, DisplayName="Positions")
	TArray<FTransform> m_Positions;
	
	void SwitchScene(const FInputActionValue& value);
	void SwitchPos(const FInputActionValue& value);
	void TakeScreenshot(const FInputActionValue& value);
	void TrackStats(const FInputActionValue& value);
	void StartSimulation(const FInputActionValue& value);

	FString GetPlayerModeString() const;
	void TakeScreenshot_Helper(int curScene, int curPos) const;
	static int32 GetWrappedIndex(int32 currentIndex, int32 increment, int32 max);
};
