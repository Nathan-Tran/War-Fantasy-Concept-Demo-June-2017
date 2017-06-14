// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WarFantasyProjectile.h"
#include "ShellCasingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class WARFANTASY_API AShellCasingProjectile : public AWarFantasyProjectile
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	class UCapsuleComponent* ShellCasingCollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class UStaticMeshComponent* ShellCasingMesh;

public:
	AShellCasingProjectile();

	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	/** Returns CollisionComp subobject **/
	//FORCEINLINE class UCapsuleComponent* GetShellCasingCollisionComp() const { return ShellCasingCollisionComp; }
	
};
