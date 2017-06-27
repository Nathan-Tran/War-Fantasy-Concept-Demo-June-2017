// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "PortalCode/EfficientPortal.h"
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
	//UPROPERTY(VisibleDefaultsOnly, Category = RotationPoint)
	//class USceneComponent* FP_ADSRotationPoint;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = SpawnPoint)
	class USceneComponent* FP_WeaponBreachLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	AEfficientPortal* activePortal;

	FRotator accumulatedRecoil, rotationSinceLastShot;

	bool bContinuousStreamOfFire = false, bRecoveringFromRecoil = false, bTriggerCurrentlyPulled = false, bFullAutoMode = true;

	// Sprint variables
	float forwardMovementSpeed = 0.f;

	// Recoil variables
	float timeUntilNextShot = 0.f, recoilRecoveryLerpAlpha = 0.f;
	float currentShotRecoilAmplifier, nextRecoilX, nextRecoilY;

	// Crouch variables
	float crouchRepositionAlpha = 0.f, crouchMovementSinceLastFrame = 0.f;

	// Lean variables
	float leanRepositionAlpha = 0.f;
	float leanRotationSinceLastFrame;
	FVector leanMovementSinceLastFrame;

	/*
		THESE ARE TWEAKABLE VALUES FOR THE CHARACTER
	*/

	float crouchCameraVerticalOffset = 55.f;
	
	/*
		THESE ARE TWEAKABLE VALUES FOR THE WEAPON RECOIL
	*/

	float ignorePlayerRecoilCompensationTolerancePitch = 1.f; // These two values might map well to the recoil values themselves
	float ignorePlayerRecoilCompensationToleranceYaw = 0.1f;
	float firstShotRecoilAmplifier = 8.f;
	float postReloadIdleTimeCompensation = 0.2f;
	int32 weaponMagazineCapacity = 30;
	int32 roundsCurrentlyInMagazine;
	FVector2D M4A1_RecoilAccuracy = FVector2D(-0.1f, -0.5f);
	FVector2D M4A1_RecoilVariance = FVector2D(-0.2f, -1.f);

	/*
		THESE ARE TWEAKABLE VALUES FOR COVER LEAN
	*/

	float leanRotationAmount = 15.f;
	FVector leanCameraTranslationOffset = FVector(0.f, 39.f, 0.f);

	/*
		PRIVATE FUNCTION DEFINITIONS
	*/

	void AddControllerPitchInputDespiteRoll(float pitch);
	void AddControllerYawInputDespiteRoll(float yaw);
	void AddPitchInputDespiteRoll(float pitch);
	void AddYawInputDespiteRoll(float yaw);

protected:
	virtual void BeginPlay();

	virtual void Tick(float DeltaTime) override;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Audio)
		class USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* FireAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	//	class UAnimMontage* HandsFireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* ADSFireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* ReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UAnimMontage* HandsReloadAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	//	class UAnimMontage* SprintAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	//	class UAnimMontage* IdleAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	//	class UAnimMontage* HandsIdleAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	//	class UAnimMontage* ADSIdleAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	//	class UAnimMontage* WalkAnimation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	//	class UAnimMontage* ADSWalkAnimation;

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

	void GivePlayerPortalReference(AEfficientPortal* activePortal);

	void lol();

};

