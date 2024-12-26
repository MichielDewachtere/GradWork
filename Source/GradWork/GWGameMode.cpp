// Fill out your copyright notice in the Description page of Project Settings.


#include "GWGameMode.h"

#include "GWPlayerController.h"
#include "GWPawn.h"

AGWGameMode::AGWGameMode()
{
	static ConstructorHelpers::FClassFinder<AGWPlayerController> playerControllerClassFinder(TEXT("/Game/BP_GWPlayerController.BP_GWPlayerController_C"));
	PlayerControllerClass = playerControllerClassFinder.Class;

	static ConstructorHelpers::FClassFinder<AGWPawn> pawnClassFinder(TEXT("/Game/BP_GWPawn.BP_GWPawn_C"));
	DefaultPawnClass = pawnClassFinder.Class;
}