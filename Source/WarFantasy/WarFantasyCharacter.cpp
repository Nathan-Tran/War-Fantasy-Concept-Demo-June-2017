// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WarFantasy.h"
#include "WarFantasyCharacter.h"
#include "WarFantasyHUD.h"
#include "ShellCasingProjectile.h"
#include "Target.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AWarFantasyCharacter

AWarFantasyCharacter::AWarFantasyCharacter()
{

	// TODO not sure if this is needed for a child of Character/Actor
	// PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(40.f, 96.0f); //TODO make capsule radius into a variable

	// set our turn rates for input
	BaseTurnRate = 45.f;				//Appears to be for controller support
	BaseLookUpRate = 45.f;				//Appears to be for controller support

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // Position the camera
	FirstPersonCameraComponent->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true; //This has to be left in to track x-axis rotation

	// Used for animating the ADS recoil
	FP_ADSRotationPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Aim Down Sights Rotation Point"));
	FP_ADSRotationPoint->SetupAttachment(FirstPersonCameraComponent);

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FP_ADSRotationPoint);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));  //TODO shrink and reposition gun
	Mesh1P->SetRelativeLocation(FVector(0.f, 0.f, -16.55f));
	Mesh1P->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh1P);
	//FP_Gun->SetupAttachment(RootComponent);

	FP_WeaponBreachLocation = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponBreachLocation"));
	FP_WeaponBreachLocation->SetupAttachment(FP_Gun);
	FP_WeaponBreachLocation->SetRelativeLocation(FVector(-1913.680420f, 3524.140625f, -961.112976f));

	// Default offset from the character location for projectiles to spawn
	//GunOffset = FVector(100.0f, 0.0f, 10.0f); //I don't think we need this

	// Enables crouching
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->MaxWalkSpeedCrouched = crouchSpeed;

	// Initialize weapon property variables (one in the chamber)
	roundsCurrentlyInMagazine = weaponMagazineCapacity + 1;

	//TODO wtf does text below mean

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

}

void AWarFantasyCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Set default walk speed. For some reason sprint is automatically called when starting the game. This issue might be solved when deriving a fresh blueprint from this class
	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("WeaponSocket"));

	Mesh1P->SetHiddenInGame(false, true);
	
}

void AWarFantasyCharacter::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);

	float currentFrameYawDelta = 0.f, currentFramePitchDelta = 0.f;
	FVector currentFrameMovementDelta = FVector::ZeroVector;

	/*
	Full Auto Fire Rate Code
	*/
	if (bTriggerCurrentlyPulled)
	{
		if (timeUntilNextShot <= 0.f) 
		{
			FireBullet();
			timeUntilNextShot = 0.075f; //TODO maybe this should just be "="
			timeUntilNextShot -= DeltaTime;
		}
		else
		{
			timeUntilNextShot -= DeltaTime;
		}
	}
	else if (timeUntilNextShot > 0.f) 
	{
		timeUntilNextShot -= DeltaTime;
	}
	else
	{
		timeUntilNextShot = 0.f;
	}

	//If semi auto enabled
	if (!bFullAutoMode) bTriggerCurrentlyPulled = false;

	if (bContinuousStreamOfFire) 
	{
		float pitchAdjust;
		float yawAdjust;

		if (bAiming) 
		{
			pitchAdjust = FMath::FInterpTo(rotationSinceLastShot.Pitch, nextRecoilY * currentShotRecoilAmplifier, DeltaTime, 40.f);
			yawAdjust = FMath::FInterpTo(rotationSinceLastShot.Yaw, nextRecoilX * currentShotRecoilAmplifier, DeltaTime, 40.f);
		}
		else
		{
			pitchAdjust = FMath::FInterpTo(rotationSinceLastShot.Pitch, (nextRecoilY / 2.f) * currentShotRecoilAmplifier, DeltaTime, 40.f);
			yawAdjust = FMath::FInterpTo(rotationSinceLastShot.Yaw, (nextRecoilX / 2.f) * currentShotRecoilAmplifier, DeltaTime, 40.f);
		}

		float pitchRecoilThisFrame = pitchAdjust - rotationSinceLastShot.Pitch;
		float yawRecoilThisFrame = yawAdjust - rotationSinceLastShot.Yaw;

		//AddPitchInputDespiteRoll(pitchRecoilThisFrame);
		//AddYawInputDespiteRoll(yawRecoilThisFrame);

		currentFramePitchDelta += pitchRecoilThisFrame;
		currentFrameYawDelta += yawRecoilThisFrame;

		accumulatedRecoil -= FRotator(pitchRecoilThisFrame, yawRecoilThisFrame, 0.f);
		rotationSinceLastShot = FRotator(pitchAdjust, yawAdjust, 0.f);

		if (FMath::IsNearlyEqual(pitchRecoilThisFrame, 0.f, 0.01f)) 
		{
			bContinuousStreamOfFire = false;
			bRecoveringFromRecoil = true;
			rotationSinceLastShot = FRotator::ZeroRotator;
		}
	}
	else if (bRecoveringFromRecoil)
	{
		FRotator rotationThisFrame = FMath::Lerp(FRotator::ZeroRotator, accumulatedRecoil, recoilRecoveryLerpAlpha) - rotationSinceLastShot;

		recoilRecoveryLerpAlpha += 10.f * DeltaTime;
		rotationSinceLastShot += rotationThisFrame;

		//AddPitchInputDespiteRoll(rotationThisFrame.Pitch);
		//AddYawInputDespiteRoll(rotationThisFrame.Yaw);

		currentFramePitchDelta += rotationThisFrame.Pitch;
		currentFrameYawDelta += rotationThisFrame.Yaw;

		if (recoilRecoveryLerpAlpha >= 1.f) {
			bRecoveringFromRecoil = false;
			recoilRecoveryLerpAlpha = 0.f;
			rotationSinceLastShot = FRotator::ZeroRotator;
			accumulatedRecoil = FRotator::ZeroRotator;
		}
	}

	// Crouch code
	if (bCrouched && crouchRepositionAlpha < 1.f) 
	{
		crouchRepositionAlpha += 10.f * DeltaTime;
		float crouchMovementThisFrame = FMath::Lerp(0.f, crouchCameraVerticalOffset, crouchRepositionAlpha) - crouchMovementSinceLastFrame;
		crouchMovementSinceLastFrame += crouchMovementThisFrame;
		//FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 0.f, -crouchMovementThisFrame));

		currentFrameMovementDelta += FVector(0.f, 0.f, -crouchMovementThisFrame);
	}
	else if (!bCrouched && crouchRepositionAlpha > 0.f) 
	{
		crouchRepositionAlpha -= 10.f * DeltaTime;
		float crouchMovementThisFrame = FMath::Lerp(0.f, crouchCameraVerticalOffset, crouchRepositionAlpha) - crouchMovementSinceLastFrame;
		crouchMovementSinceLastFrame += crouchMovementThisFrame;
		//FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 0.f, -crouchMovementThisFrame));

		currentFrameMovementDelta += FVector(0.f, 0.f, -crouchMovementThisFrame);
	}

	// Cover lean code
	if (bLeaningRight && leanRepositionAlpha < 1.f)
	{
		leanRepositionAlpha += 10.f * DeltaTime;;

		FVector leanMovementThisFrame = FMath::Lerp(FVector::ZeroVector, leanCameraTranslationOffset, leanRepositionAlpha) - leanMovementSinceLastFrame;
		float leanRotationThisFrame = FMath::Lerp(0.f, leanRotationAmount, leanRepositionAlpha) - leanRotationSinceLastFrame;
		leanMovementSinceLastFrame += leanMovementThisFrame;
		leanRotationSinceLastFrame += leanRotationThisFrame;
		//FirstPersonCameraComponent->AddRelativeLocation(leanMovementThisFrame);
		currentFrameMovementDelta += leanMovementThisFrame;
		AddControllerRollInput(leanRotationThisFrame);
	}
	else if (bLeaningLeft && leanRepositionAlpha > -1.f)
	{
		leanRepositionAlpha -= 10.f * DeltaTime;

		FVector leanMovementThisFrame = FMath::Lerp(FVector::ZeroVector, -leanCameraTranslationOffset, -leanRepositionAlpha) - leanMovementSinceLastFrame;
		float leanRotationThisFrame = FMath::Lerp(0.f, -leanRotationAmount, -leanRepositionAlpha) - leanRotationSinceLastFrame;
		leanMovementSinceLastFrame += leanMovementThisFrame;
		leanRotationSinceLastFrame += leanRotationThisFrame;
		//FirstPersonCameraComponent->AddRelativeLocation(leanMovementThisFrame);
		currentFrameMovementDelta += leanMovementThisFrame;
		AddControllerRollInput(leanRotationThisFrame);
	} 
	else if (!bLeaningRight && leanRepositionAlpha > 0.f)
	{
		leanRepositionAlpha -= 10.f * DeltaTime;
		if (leanRepositionAlpha < 0.f) leanRepositionAlpha = 0.f;

		FVector leanMovementThisFrame = FMath::Lerp(FVector::ZeroVector, leanCameraTranslationOffset, leanRepositionAlpha) - leanMovementSinceLastFrame;
		float leanRotationThisFrame = FMath::Lerp(0.f, leanRotationAmount, leanRepositionAlpha) - leanRotationSinceLastFrame;
		leanMovementSinceLastFrame += leanMovementThisFrame;
		leanRotationSinceLastFrame += leanRotationThisFrame;
		//FirstPersonCameraComponent->AddRelativeLocation(leanMovementThisFrame);
		currentFrameMovementDelta += leanMovementThisFrame;
		AddControllerRollInput(leanRotationThisFrame);
	}
	else if(!bLeaningLeft && leanRepositionAlpha < 0.f)
	{
		leanRepositionAlpha += 10.f * DeltaTime;
		if (leanRepositionAlpha > 0.f) leanRepositionAlpha = 0.f;

		FVector leanMovementThisFrame = FMath::Lerp(FVector::ZeroVector, -leanCameraTranslationOffset, -leanRepositionAlpha) - leanMovementSinceLastFrame;
		float leanRotationThisFrame = FMath::Lerp(0.f, -leanRotationAmount, -leanRepositionAlpha) - leanRotationSinceLastFrame;
		leanMovementSinceLastFrame += leanMovementThisFrame;
		leanRotationSinceLastFrame += leanRotationThisFrame;
		//FirstPersonCameraComponent->AddRelativeLocation(leanMovementThisFrame);
		currentFrameMovementDelta += leanMovementThisFrame;
		AddControllerRollInput(leanRotationThisFrame);
	}

	if (currentFramePitchDelta != 0.f && currentFrameYawDelta != 0.f)
	{
		AddPitchInputDespiteRoll(currentFramePitchDelta);
		AddYawInputDespiteRoll(currentFrameYawDelta);
	}

	if (currentFrameMovementDelta != FVector::ZeroVector)
		FirstPersonCameraComponent->AddRelativeLocation(currentFrameMovementDelta);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWarFantasyCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AWarFantasyCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AWarFantasyCharacter::StopSprint);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWarFantasyCharacter::OnFireDown);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AWarFantasyCharacter::OnFireUp);

	PlayerInputComponent->BindAction("Switch Fire Mode", IE_Pressed, this, &AWarFantasyCharacter::SwitchFireMode);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AWarFantasyCharacter::OnReload);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AWarFantasyCharacter::OnLookDownSights);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AWarFantasyCharacter::OnLookAwayFromSights);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AWarFantasyCharacter::OnCrouchDown);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AWarFantasyCharacter::OnStandUp);

	PlayerInputComponent->BindAction("Lean Right", IE_Pressed, this, &AWarFantasyCharacter::LeanRight);
	PlayerInputComponent->BindAction("Lean Right", IE_Released, this, &AWarFantasyCharacter::StandStraight);

	PlayerInputComponent->BindAction("Lean Left", IE_Pressed, this, &AWarFantasyCharacter::LeanLeft);
	PlayerInputComponent->BindAction("Lean Left", IE_Released, this, &AWarFantasyCharacter::StandStraight);

	PlayerInputComponent->BindAxis("MoveForward", this, &AWarFantasyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWarFantasyCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &AWarFantasyCharacter::AddControllerYawInputDespiteRoll);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWarFantasyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AWarFantasyCharacter::AddControllerPitchInputDespiteRoll);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWarFantasyCharacter::LookUpAtRate);
}

/** If the player's input is within a ceratin range, then use it to compensate for recoil*/
void AWarFantasyCharacter::AddControllerPitchInputDespiteRoll(float pitch)
{
	if (accumulatedRecoil != FRotator::ZeroRotator && pitch < ignorePlayerRecoilCompensationTolerancePitch)
	{
		accumulatedRecoil -= FRotator(pitch, 0.f, 0.f);
	}
	AddPitchInputDespiteRoll(pitch);
}

/** If the player's input is within a ceratin range, then use it to compensate for recoil*/
void AWarFantasyCharacter::AddControllerYawInputDespiteRoll(float yaw)
{
	if (accumulatedRecoil != FRotator::ZeroRotator && FMath::Abs(yaw) < ignorePlayerRecoilCompensationToleranceYaw)
	{
		accumulatedRecoil -= FRotator(0.f, yaw, 0.f);
	}
	AddYawInputDespiteRoll(yaw);
}

void AWarFantasyCharacter::AddPitchInputDespiteRoll(float pitch)
{
	float roll = FirstPersonCameraComponent->RelativeRotation.Roll;
	//float finalPitch = pitch, finalYaw = 0;

	//if (roll > 0.1) 
	//{
	float finalPitch = pitch * FMath::Cos(FMath::DegreesToRadians(roll));
	float finalYaw = -1.f * pitch * FMath::Sin(FMath::DegreesToRadians(roll));
	//}
	//else if (roll < -0.1)
	//{
	//	finalPitch = pitch * FMath::Cos(FMath::DegreesToRadians(roll));
	//	finalYaw = -1.f * pitch * FMath::Sin(FMath::DegreesToRadians(roll));
	//}

	AddControllerPitchInput(finalPitch);
	AddControllerYawInput(finalYaw);
}

void AWarFantasyCharacter::AddYawInputDespiteRoll(float yaw)
{
	float roll = FirstPersonCameraComponent->RelativeRotation.Roll;
	//float finalPitch = 0, finalYaw = yaw;

	//if (roll > 0.1)
	//{
		//finalPitch = -1.f * yaw * FMath::Sin(roll);
		//finalYaw = yaw * FMath::Cos(roll);
	//}
	//else if (roll < -0.1)
	//{
		float finalPitch = -1.f * yaw * FMath::Sin(FMath::DegreesToRadians(roll));
		float finalYaw = yaw * FMath::Cos(FMath::DegreesToRadians(roll));
	//}

	AddControllerPitchInput(finalPitch);
	AddControllerYawInput(finalYaw);
}

/** Gun rigger pulled. */
void AWarFantasyCharacter::OnFireDown()
{
	//if (bReloading) return;
	bTriggerCurrentlyPulled = true;
	currentShotRecoilAmplifier = firstShotRecoilAmplifier;
}

/** Gun trigger released */
void AWarFantasyCharacter::OnFireUp()
{
	bTriggerCurrentlyPulled = false;
}

void AWarFantasyCharacter::FireBullet()
{
	// These are the correct scaling values
	/*InputYawScale = 2.5;
	InputPitchScale = -1.75;*/


	// Reduce the initial shot recoil
	currentShotRecoilAmplifier = FMath::Sqrt(currentShotRecoilAmplifier);

	if (roundsCurrentlyInMagazine == 0)
	{
		//TODO play empty chamber sounds
		//TODO decide if I should force them to reload
		UE_LOG(LogTemp, Warning, TEXT("Empty Mag"));
		bTriggerCurrentlyPulled = false;
		return;
	}

	// try and fire a projectile
	if (ShellCasing != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation() + FRotator(30.f, 0.f, 90.f);
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			
			// I don't think we need this redundency
			//const FVector SpawnLocation = ((FP_WeaponBreachLocation != nullptr) ? FP_WeaponBreachLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
			const FVector SpawnLocation = FP_WeaponBreachLocation->GetComponentLocation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding; //TODO set spawn params for shell casings

			// spawn the projectile at the muzzle
			World->SpawnActor<AShellCasingProjectile>(ShellCasing, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	/*
	*	RayCast code
	*/
	FHitResult* bulletHit = new FHitResult();
	FVector startTrace;
	FVector endTrace;

	// Aim the ray from the camera center or from the gun barrel
	if (!bAiming) {
		startTrace = FirstPersonCameraComponent->GetComponentLocation();
		endTrace = (FirstPersonCameraComponent->GetForwardVector() * 6000.f) + startTrace;
	}
	else
	{
		startTrace = FP_Gun->GetComponentLocation();
		endTrace = (FP_Gun->GetRightVector() * 6000.f) + startTrace;
	}

	FCollisionQueryParams* traceParams = new FCollisionQueryParams();

	DrawDebugLine(GetWorld(), startTrace, endTrace, FColor::Red, false, 50.0f);

	if (GetWorld()->LineTraceSingleByChannel(*bulletHit, startTrace, endTrace, ECC_Visibility, *traceParams)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("PEWWWW PEWWW Hit SOmething"));

		ATarget* possibleTarget = Cast<ATarget>(bulletHit->Actor.Get());

		if (possibleTarget != NULL && !possibleTarget->IsPendingKill()) 
		{
			possibleTarget->DamageTarget(100.f);
		}
	}


	// Calculate the next shot's recoil
	float randY = FMath::FRandRange(0.f, 1.f);
	nextRecoilY = (0.5f + randY) * M4A1_RecoilAccuracy.Y;
	nextRecoilX = (0.5f + FMath::FRandRange(0.f, ((1.f - randY) / 1.5f))) * M4A1_RecoilAccuracy.X;

	// Get the animation object for the arms mesh
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();

	// try and play a firing animation if specified
	if (!bAiming && FireAnimation != NULL)
	{
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}

		if (bRecoveringFromRecoil) {

			accumulatedRecoil *= (1.f - recoilRecoveryLerpAlpha);
			recoilRecoveryLerpAlpha = 0.f;
		}

		bContinuousStreamOfFire = true;
		rotationSinceLastShot = FRotator::ZeroRotator;
	} 
	else if (bAiming && ADSFireAnimation != NULL)
	{
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(ADSFireAnimation, 1.f);
			//TODO perhaps play both gun and hand animations here
		}

		if (bRecoveringFromRecoil) {

			accumulatedRecoil *= (1.f - recoilRecoveryLerpAlpha);
			recoilRecoveryLerpAlpha = 0.f;
		}

		bContinuousStreamOfFire = true;
		rotationSinceLastShot = FRotator::ZeroRotator;
	}

	roundsCurrentlyInMagazine -= 1;
}

void AWarFantasyCharacter::StartSprint()
{
	StandStraight();

	bSprinting = true;

	GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;

}

void AWarFantasyCharacter::StopSprint()
{
	bSprinting = false;

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
}

void AWarFantasyCharacter::SwitchFireMode()
{
	bFullAutoMode = !bFullAutoMode;
}

void AWarFantasyCharacter::OnReload()
{
	// try and play a firing animation if specified
	if (HandsReloadAnimation != NULL && ReloadAnimation != NULL)
	{
		bReloading = true;

		// Get the animation object for the arms mesh
		UAnimInstance* HandsAnimInstance = Mesh1P->GetAnimInstance();
		UAnimInstance* GunAnimInstance = FP_Gun->GetAnimInstance();
		if (HandsAnimInstance != NULL && GunAnimInstance != NULL)
		{
			//Play the reload animations for the gun and the hand
			HandsAnimInstance->Montage_Play(HandsReloadAnimation, 1.f);
			GunAnimInstance->Montage_Play(ReloadAnimation, 1.f);
			//GunAnimInstance->GetRelevantAnimLength

			//FTimerHandle PostAnimTimerHandle;
			//GetWorldTimerManager().SetTimer(PostAnimTimerHandle, this, bReloading = false, GunAnimInstance->GetRelevantAnimLength(0,0), false);

			//TODO delay code (I'm starting to think all animations should be done through blueprints)

			if (roundsCurrentlyInMagazine > 0) roundsCurrentlyInMagazine = 1;
			roundsCurrentlyInMagazine += weaponMagazineCapacity;
		}
	}

	
}

void AWarFantasyCharacter::OnLookDownSights()
{
	if (!bSprinting)
		bAiming = true;

	AWarFantasyHUD * HUD = Cast<AWarFantasyHUD>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
	HUD->ToggleHud();

	//TODO: Camera zoom

	GetCharacterMovement()->MaxWalkSpeed = ADSWalkSpeed;
}

void AWarFantasyCharacter::OnLookAwayFromSights()
{
	bAiming = false;
	bRecoveringFromRecoil = false;

	AWarFantasyHUD * HUD = Cast<AWarFantasyHUD>(UGameplayStatics::GetPlayerController(this, 0)->GetHUD());
	HUD->ToggleHud();

	//FP_ADSRotationPoint->SetRelativeRotation(firstShotRotation);

	//TODO: undo camera zoom

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed; //TODO this logic ain't gon play
}

/** Cease ADS. */
void AWarFantasyCharacter::OnCrouchDown()
{
	UE_LOG(LogTemp, Warning, TEXT("CROUCH PRESSED"));
	bCrouched = true;

	//FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 0.f); // Position the camera

	//FVector tempStorage = FirstPersonCameraComponent->GetComponentLocation();
	FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 0.f, crouchCameraVerticalOffset));
	Crouch();

}

/** Cease ADS. */
void AWarFantasyCharacter::OnStandUp()
{
	bCrouched = false; //TODO possibly unused

	FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 0.f, -crouchCameraVerticalOffset));
	UnCrouch();
}

/** Enable player lean right */
void AWarFantasyCharacter::LeanRight()
{
	if (bLeaningLeft || bSprinting) return;
	bLeaningRight = true;
}

/** Enable player lean right */
void AWarFantasyCharacter::LeanLeft()
{
	if (bLeaningRight || bSprinting) return;
	bLeaningLeft = true;
}

/** Disable player lean */
void AWarFantasyCharacter::StandStraight()
{
	bLeaningRight = false, bLeaningLeft = false;
}

void AWarFantasyCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AWarFantasyCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		FVector movementVector = GetActorRightVector();
		//if (bSprinting) movementVector *= sprintRate;

		// add movement in that direction
		AddMovementInput(movementVector, Value);
	}
}

void AWarFantasyCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWarFantasyCharacter::LookUpAtRate(float Rate)
{
	//TODO Fix aiming when leaning
	/*
	if(roll != 0) {
		Crazy rotation math
	} else {
		Do the shit below
	}
	*/
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

//TODO aim should be affected by velocity
//TODO perhaps movement speeds depend on direction
//TODO can only sprint forward
//TODO can't fire when sprinting or reloading
//TODO add croach/lean lerp/slerp
//TODO lock animations when reloading
//TODO re-implement gun firing and mussle flash animation
//TODO factor gun rotation into shell casing ejection angle
//TODO fix bug where shell casings collide with gun and cause a phsyics impact
//TODO remove dependance on blueprint to change camera/mesh scale
//TODO fix the max vertical aim + automatic fire bug
//TODO sprint interupts reload
//TODO refactor code to make it modular
//TODO make the arms mesh slide back and forth while firing
//TODO first few shots in autofire should have more recoil
//TODO for all lerp and interpto functions, deltatime must be implemented
//TODO shell casing projection should inherit the player velocity
//TODO depth of field effect

