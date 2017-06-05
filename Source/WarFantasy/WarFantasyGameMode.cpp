// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WarFantasy.h"
#include "WarFantasyGameMode.h"
#include "WarFantasyHUD.h"
#include "WarFantasyCharacter.h"

AWarFantasyGameMode::AWarFantasyGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWarFantasyHUD::StaticClass();
}
