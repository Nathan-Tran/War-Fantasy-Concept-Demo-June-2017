// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "WarFantasyGameMode.generated.h"

UCLASS(minimalapi)
class AWarFantasyGameMode : public AGameModeBase
{
	GENERATED_BODY()


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	AWarFantasyGameMode();
};



