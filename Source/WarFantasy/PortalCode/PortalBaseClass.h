// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
//#include "WarFantasyCharacter.h"
#include "PortalBaseClass.generated.h"

UCLASS()
class WARFANTASY_API APortalBaseClass : public AActor
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	APortalBaseClass();

protected:

	class APlayerCameraManager* playerCamera;

	ACharacter* thePlayer;

	UPROPERTY(VisibleDefaultsOnly)
		class USceneComponent* PortalA;

	UPROPERTY(VisibleDefaultsOnly)
		class USceneComponent* PortalB;

	/** The mesh for the doorway */
	UPROPERTY(EditInstanceOnly)
		class UStaticMeshComponent* PortalDoorway;

	/** The camera perspective of the doorway */
	UPROPERTY(VisibleDefaultsOnly)
		class USceneCaptureComponent2D* PortalCapture;

	// Called every frame
	//virtual void Tick(float DeltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PositionClippingPlane();

	UFUNCTION(BlueprintCallable)
		void SetPortalBLocationAndRotation(FVector location, FRotator rotation);

	UFUNCTION(BlueprintCallable)
		void SetPortalScale(FVector portalScale, FVector portalLocation);
};
