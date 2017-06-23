// Fill out your copyright notice in the Description page of Project Settings.

#include "WarFantasy.h"
#include "EfficientPortal.h"

// Sets default values
AEfficientPortal::AEfficientPortal()
{
	//Super::APortalBaseClass();

	PortalB = CreateDefaultSubobject<USceneComponent>(TEXT("Portal B Location"));

	PortalCapture->SetupAttachment(PortalB);

	UWorld* const pWorld = GetWorld();
	//auto pWorld = GetWorld();
	if (pWorld)
	{
		playerCamera = Cast<UCameraComponent>(UGameplayStatics::GetPlayerCameraManager(pWorld, 0));
		//UGameplayStatics::GetPlayerCameraManager
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Fuck you"));
	}
}

// Called every frame
void AEfficientPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!playerCamera) {
		UE_LOG(LogTemp, Warning, TEXT("Osdfas"));
		return;
	}

	UpdatePortalView();
}

// Called when the game starts or when spawned
void AEfficientPortal::BeginPlay()
{
	Super::BeginPlay();

}

// Called when the game starts or when spawned
void AEfficientPortal::SwapPortalLocations()
{
	FTransform tempTransfrom = PortalA->GetComponentTransform();
	PortalA->SetWorldTransform(PortalB->GetComponentTransform());
	PortalB->SetWorldTransform(tempTransfrom);
}

// Called when the game starts or when spawned
void AEfficientPortal::UpdatePortalView()
{
	FRotator newRotation = PortalB->GetComponentRotation() - (FRotator(0.f, 0.f, 180.f) + PortalA->GetComponentRotation());
	FVector newLocation = PortalB->GetComponentLocation() - (PortalA->GetComponentLocation().RotateAngleAxis(newRotation.Roll, FVector(0.f, 0.f, 1.f))) + (playerCamera->GetComponentLocation().RotateAngleAxis(newRotation.Roll, FVector(0.f, 0.f, 1.f)));
	newRotation += playerCamera->GetComponentRotation();
	PortalCapture->SetWorldLocationAndRotation(newLocation, newRotation);
}


