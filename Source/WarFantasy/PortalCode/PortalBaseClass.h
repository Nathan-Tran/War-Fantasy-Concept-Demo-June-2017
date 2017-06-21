// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PortalBaseClass.generated.h"

UCLASS()
class WARFANTASY_API APortalBaseClass : public AActor
{
	GENERATED_BODY()

	class USceneComponent* DefaultSceneComponent;

	/** The mesh for the doorway */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* PortalDoorway;

	/** The camera perspective of the doorway */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneCaptureComponent2D* PortalCapture;

	// Sets default values for this actor's properties
	APortalBaseClass();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
	
};
