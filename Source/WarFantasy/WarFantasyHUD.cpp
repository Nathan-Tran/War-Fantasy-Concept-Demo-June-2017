// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WarFantasy.h"
#include "WarFantasyHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"

AWarFantasyHUD::AWarFantasyHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshiarTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshiarTexObj.Object;

	// find center of the Canvas
	//Center = FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	//CrosshairDrawPosition = FVector2D((Center.X - (CrosshairTex->GetSurfaceWidth() / 2)),
	//	(Center.Y - (CrosshairTex->GetSurfaceHeight() / 2)));
}


void AWarFantasyHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	const FVector2D Center = FVector2D(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition = FVector2D((Center.X - (CrosshairTex->GetSurfaceWidth() / 2)),
		(Center.Y - (CrosshairTex->GetSurfaceHeight() / 2)));

	if (bEnableHUD) 
	{
		// draw the crosshair
		FCanvasTileItem TileItem(CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
		TileItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(TileItem);
	}
}


/** Enables and disables HUD*/
void AWarFantasyHUD::ToggleHud()
{
	if (bEnableHUD) bEnableHUD = false;
	else bEnableHUD = true;
}

