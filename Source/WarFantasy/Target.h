// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Target.generated.h"

UCLASS()
class WARFANTASY_API ATarget : public AActor
{
	GENERATED_BODY()

	float health = 100;

	// The mesh for the target
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* targetMesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	ATarget();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the target is damaged
	void DamageTarget(float damage);

	// Called on destruction of target
	void DestroyTarget();
	
};
