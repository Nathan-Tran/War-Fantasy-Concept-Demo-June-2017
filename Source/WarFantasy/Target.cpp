// Fill out your copyright notice in the Description page of Project Settings.

#include "WarFantasy.h"
#include "Target.h"


// Sets default values
ATarget::ATarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	targetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Target Mesh"));
	RootComponent = targetMesh;

}

// Called when the game starts or when spawned
void ATarget::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called when the target is damaged
void  ATarget::DamageTarget(float damage)
{
	health -= damage;

	if (health <= 0)
	{
		DestroyTarget();
	}
}

// Called on destruction of target
void  ATarget::DestroyTarget()
{
	Destroy();
}

