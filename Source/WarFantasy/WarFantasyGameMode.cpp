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



// Called when the game starts or when spawned
void AWarFantasyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine)
	{
		//TODO
		UGameUserSettings* MyGameSettings = GEngine->GetGameUserSettings();
		MyGameSettings->SetScreenResolution(FIntPoint(1920, 1080));
		MyGameSettings->SetFullscreenMode(EWindowMode::Fullscreen);
		MyGameSettings->SetVSyncEnabled(true);
		MyGameSettings->ApplySettings(false);
		//See if you can disable the gpu buffer from here
	}
}
