// Fill out your copyright notice in the Description page of Project Settings.

#include "WarFantasy.h"
#include "EfficientPortal.h"
#include "WarFantasyCharacter.h"

// Sets default values
AEfficientPortal::AEfficientPortal()
{
	PortalB = CreateDefaultSubobject<USceneComponent>(TEXT("Portal B Location"));
	PortalB->SetWorldLocation(PortalA->GetComponentLocation());

	PortalCapture->SetupAttachment(PortalB);
}

void AEfficientPortal::TeleportPlayerIfNecessary(float DeltaTime)
{
	if (IsOverlappingActor(thePlayer))
	{
		FVector playerVelocity = thePlayer->GetVelocity();
		FVector temp = PortalDoorway->GetComponentLocation() - (thePlayer->GetActorLocation() + (playerVelocity * DeltaTime));
		FRotator portalARotation = PortalA->GetComponentRotation();
		temp.Z = 0;
		temp = portalARotation.UnrotateVector(temp);

		//return (temp.X >= 0 && temp.X <= 100);

		if (temp.X >= 0 && temp.X <= 100)
		{
			UE_LOG(LogTemp, Warning, TEXT("SHOULD TELEPORT NOW"));

			// Set teleportation destination
			FRotator portalBRotation = PortalB->GetComponentRotation();
			FVector destination = portalBRotation.RotateVector(temp) + PortalB->GetComponentLocation();

			// Set teleportation rotation
			FRotator finalRotation = (thePlayer->GetController()->GetControlRotation() - (portalARotation + FRotator(0.f, 180.f, 0.f))) + portalBRotation;

			// Teleport (The ordering of rotating and teleporting the player might have an effect on the white flash of the transition
			thePlayer->GetController()->SetControlRotation(finalRotation);
			thePlayer->TeleportTo(destination, FRotator::ZeroRotator); // TeleportTo() instead of SetActorLocation() might prevent the bright flash

																	   // Velocity change compensation
			FVector newVelocity = ((portalBRotation - portalARotation) + FRotator(0.f, 180.f, 0.f)).RotateVector(playerVelocity);
			thePlayer->GetMovementComponent()->Velocity = newVelocity;

			SwapPortalLocations();
			PositionClippingPlane();
		}
	}
}

FTransform AEfficientPortal::GetNewPlayerTransform(FTransform currentPlayerTransform)
{
	return FTransform::Identity;
}

// Called every frame
void AEfficientPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//TeleportPlayer(DeltaTime);
	UpdatePortalView();
}

// Called when the game starts or when spawned
void AEfficientPortal::BeginPlay()
{
	Super::BeginPlay();

	AWarFantasyCharacter* tempPlayerReference = Cast<AWarFantasyCharacter>(thePlayer);

	if (tempPlayerReference)
	{
		tempPlayerReference->GivePlayerPortalReference(this);
	}
}

// Called when the game starts or when spawned
void AEfficientPortal::SwapPortalLocations()
{
	FTransform tempTransfrom = PortalA->GetComponentTransform();
	PortalA->SetWorldTransform(PortalB->GetComponentTransform());
	PortalB->SetWorldTransform(tempTransfrom);
}

// Called to check if the player is colliding with the portal
/*void AEfficientPortal::TeleportPlayer(float DeltaTime)
{
	if (IsOverlappingActor(thePlayer))
	{
		return;
		FVector playerVelocity = thePlayer->GetVelocity();
		FVector temp = PortalDoorway->GetComponentLocation() - (thePlayer->GetActorLocation() + (playerVelocity * DeltaTime));
		FRotator portalARotation = PortalA->GetComponentRotation();
		temp.Z = 0;
	//rotate
		temp = portalARotation.UnrotateVector(temp);

		if (temp.X >= 0 && temp.X <= 100)
		{
			UE_LOG(LogTemp, Warning, TEXT("SHOULD TELEPORT NOW"));

			// Set teleportation destination
			FRotator portalBRotation = PortalB->GetComponentRotation();
			FVector destination = portalBRotation.RotateVector(temp) + PortalB->GetComponentLocation();
			//thePlayer->TeleportTo(destination, portalBRotation);

			// Move the portal location
			//PortalDoorway->SetWorldLocationAndRotationNoPhysics(PortalB->GetComponentLocation() + FVector(0.f, 0.f, 100.f/*Portal half height*//*), portalBRotation + FRotator(-90.f, 0.f, 0.f));
			//I should change the parents of portalcapture portaldoorway (unless it causes bugs)
			//lol = true;
			//PositionClippingPlane();

			// Set teleportation rotation
			FRotator finalRotation = (thePlayer->GetController()->GetControlRotation() - (portalARotation + FRotator(0.f, 180.f, 0.f))) + portalBRotation;

			// Teleport (The ordering of rotating and teleporting the player might have an effect on the white flash of the transition
			thePlayer->GetController()->SetControlRotation(finalRotation);
			thePlayer->TeleportTo(destination, FRotator::ZeroRotator); // TeleportTo() instead of SetActorLocation() might prevent the bright flash
			
			// Velocity change compensation
			FVector newVelocity = ((portalBRotation - portalARotation) + FRotator(0.f, 180.f, 0.f)).RotateVector(playerVelocity);
			thePlayer->GetMovementComponent()->Velocity = newVelocity;

			SwapPortalLocations();
			PositionClippingPlane();
		}
	}
}*/

// Called each frame to repositon the sceneCapture2D
void AEfficientPortal::UpdatePortalView()
{
	FRotator newRotation = PortalB->GetComponentRotation() - (FRotator(0.f, 180.f, 0.f) + PortalA->GetComponentRotation());
	FVector newLocation = PortalB->GetComponentLocation() - (PortalA->GetComponentLocation().RotateAngleAxis(newRotation.Yaw, FVector(0.f, 0.f, 1.f))) + (playerCamera->GetCameraLocation().RotateAngleAxis(newRotation.Yaw, FVector(0.f, 0.f, 1.f)));
	newRotation += playerCamera->GetCameraRotation();
	PortalCapture->SetWorldLocationAndRotation(newLocation, newRotation);
}


