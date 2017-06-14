// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "WarFantasyHUD.generated.h"

UCLASS()
class AWarFantasyHUD : public AHUD
{
	GENERATED_BODY()

	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

	bool bEnableHUD = true;

	//FVector2D Center, CrosshairDrawPosition;

public:
	AWarFantasyHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	/** Enables and disables HUD*/
	void ToggleHud();

};

