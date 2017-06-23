// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "WarFantasyCharacter.h"
#include "PortalBaseClass.generated.h"

UCLASS()
class WARFANTASY_API APortalBaseClass : public AActor
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	APortalBaseClass();

protected:

	class USceneComponent* PortalA;

	class UCameraComponent* playerCamera;

	/** The mesh for the doorway */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class UStaticMeshComponent* PortalDoorway;

	/** The camera perspective of the doorway */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
		class USceneCaptureComponent2D* PortalCapture;

	// Called every frame
	/*virtual void Tick(float DeltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;*/
};
