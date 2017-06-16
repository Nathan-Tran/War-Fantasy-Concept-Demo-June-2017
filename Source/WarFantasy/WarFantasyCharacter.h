// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "WarFantasyCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AWarFantasyCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Used for animating the ADS recoil */
	UPROPERTY(VisibleDefaultsOnly, Category = RotationPoint)
	class USceneComponent* FP_ADSRotationPoint;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = SpawnPoint)
	class USceneComponent* FP_WeaponBreachLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	FRotator recoilRotation, firstShotRotation, accumulatedRecoil, rotationSinceLastShot;

	bool bContinuousStreamOfFire = false, bRecoveringFromRecoil = false, bTriggerCurrentlyPulled = false, bFullAutoMode = true;

	void AddControllerPitchInputDespiteRoll(float pitch);
	void AddControllerYawInputDespiteRoll(float yaw);
	void AddPitchInputDespiteRoll(float pitch);
	void AddYawInputDespiteRoll(float yaw);

	// Recoil variables
	float timeUntilNextShot = 0.f, recoilRecoveryLerpAlpha = 0.f;

	// Crouch variables
	float crouchRepositionAlpha = 0.f, crouchMovementSinceLastFrame = 0.f;

	/*
		THESE ARE TWEAKABLE VALUES FOR THE CHARACTER
	*/

	float crouchCameraVerticalOffset = 55.f;
	
	/*
		THESE ARE TWEAKABLE VALUES FOR THE WEAPON RECOIL
	*/

	float ignorePlayerRecoilCompensationTolerancePitch = 1.f;
	float ignorePlayerRecoilCompensationToleranceYaw = 0.5f;

	/*
		THESE ARE TWEAKABLE VALUES FOR COVER LEAN
	*/

	float leanRotationAmount = 25.f;

protected:
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* HandsFireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* ADSFireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* ReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* HandsReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* SprintAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* HandsIdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* ADSIdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* WalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* ADSWalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* RaiseAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* LowerAnimation;

	/** Gun rigger pulled. */
	void OnFireDown();

	/** Gun trigger released */
	void OnFireUp();

	/** Fires a ray and plays animations. */
	void FireBullet();

	/** Initiates the sprint movement and animations. */
	void StartSprint();

	/** Cease the sprint movement and animations. */
	void StopSprint();

	/** Switch weapon from auto to semi. */
	void SwitchFireMode();

	/** Reloads the weapon. */
	void OnReload();

	/** Initiate ADS. */
	void OnLookDownSights();

	/** Cease ADS. */
	void OnLookAwayFromSights();

	/** Enabled player crouch */
	void OnCrouchDown();

	/** Disable player crouch */
	void OnStandUp();

	/** Enable player lean right */
	void LeanRight();

	/** Enable player lean left */
	void LeanLeft();

	/** Disable player lean */
	void StandStraight();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	bool bCrouched = false, bLeaningRight = false, bLeaningLeft = false;

public:
	AWarFantasyCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
		float BaseLookUpRate;

	const float sprintSpeed = 900.f;
	const float walkSpeed = 600.f;
	const float crouchSpeed = 300.f;
	const float ADSWalkSpeed = 200.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
		bool bReloading = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
		bool bSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
		bool bAiming = false;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
		FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
		TSubclassOf<class AShellCasingProjectile> ShellCasing;

	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

