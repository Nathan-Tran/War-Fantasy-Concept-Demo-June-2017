// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PortalCode/PortalBaseClass.h"
#include "EfficientPortal.generated.h"

/**
 * 
 */
UCLASS()
class WARFANTASY_API AEfficientPortal : public APortalBaseClass
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, Category = OtherPortalTransform)
	class USceneComponent* PortalB;

public:

	AEfficientPortal();

protected:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SwapPortalLocations();

	void UpdatePortalView();
};
