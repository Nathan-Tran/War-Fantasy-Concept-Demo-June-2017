// Fill out your copyright notice in the Description page of Project Settings.

#include "WarFantasy.h"
#include "ShellCasingProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

AShellCasingProjectile::AShellCasingProjectile()
{
	// Use a sphere as a simple collision representation
	ShellCasingCollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	ShellCasingCollisionComp->InitCapsuleSize(2.5f, 0.6f);
	ShellCasingCollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	ShellCasingCollisionComp->OnComponentHit.AddDynamic(this, &AWarFantasyProjectile::OnHit);		// set up a notification for when this component hits something blocking																					
	ShellCasingCollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));	// Players can't walk on it
	ShellCasingCollisionComp->CanCharacterStepUpOn = ECB_No;
	//ShellCasingCollisionComp->SetSimulatePhysics(true);

	// Set as root component
	RootComponent = ShellCasingCollisionComp;

	ShellCasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellCasing"));
	ShellCasingMesh->SetupAttachment(ShellCasingCollisionComp);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Game/IBFPSAmmo/Ammo/ammo_556x45_case"));
	if (MeshObj.Object)
		ShellCasingMesh->SetStaticMesh(MeshObj.Object);
	ShellCasingMesh->RelativeLocation = FVector(0.f, 0.f, -1.0f);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement->UpdatedComponent = ShellCasingCollisionComp;
	ProjectileMovement->InitialSpeed = 400.f;
	ProjectileMovement->MaxSpeed = 400.f;

	// Die after 5 seconds by default
	InitialLifeSpan = 5.0f;
}

void AShellCasingProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//TODO: Play Sound
}


