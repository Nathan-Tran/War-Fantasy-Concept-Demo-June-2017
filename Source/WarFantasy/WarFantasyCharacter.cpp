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
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(40.f, 96.0f); //TODO make capsule radius into a variable

	// set our turn rates for input
	BaseTurnRate = 45.f;				//Appears to be for controller support
	BaseLookUpRate = 45.f;				//Appears to be for controller support

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 64.f); // Position the camera
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
	Mesh1P->RelativeRotation = FRotator(0.f, -90.f, 0.f);  //TODO shrink and reposition gun
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -16.55f);
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

	/*
	Full Auto Fire Rate Code
	*/
	if (bTriggerCurrentlyPulled)
	{

		if (timeUntilNextShot <= 0.f) 
		{
			timeUntilNextShot = 0.075f;//TODO maybe this should just be "="
			FireBullet();
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
			pitchAdjust = FMath::FInterpTo(rotationSinceLastShot.Pitch, -2.f, DeltaTime, 40.f);
			yawAdjust = FMath::FInterpTo(rotationSinceLastShot.Yaw, -0.2f, DeltaTime, 40.f);
		}
		else
		{
			pitchAdjust = FMath::FInterpTo(rotationSinceLastShot.Pitch, -1.f, DeltaTime, 40.f);
			yawAdjust = FMath::FInterpTo(rotationSinceLastShot.Yaw, -0.1f, DeltaTime, 40.f);
		}

		float pitchRecoilThisFrame = pitchAdjust - rotationSinceLastShot.Pitch;
		float yawRecoilThisFrame = yawAdjust - rotationSinceLastShot.Yaw;

		AddPitchInputDespiteRoll(pitchRecoilThisFrame);
		AddYawInputDespiteRoll(yawRecoilThisFrame);
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

		recoilRecoveryLerpAlpha += 0.1f;

		rotationSinceLastShot += rotationThisFrame;
		AddPitchInputDespiteRoll(rotationThisFrame.Pitch);
		AddYawInputDespiteRoll(rotationThisFrame.Yaw);

		if (recoilRecoveryLerpAlpha >= 1.f) {
			UE_LOG(LogTemp, Warning, TEXT("RECOIL RECOVERY COMPLETE"));
			bRecoveringFromRecoil = false;
			recoilRecoveryLerpAlpha = 0.f;
			rotationSinceLastShot = FRotator::ZeroRotator;
			accumulatedRecoil = FRotator::ZeroRotator;
		}
	}

	//TODO REMOVEOLD VARIABLES FROM HEADER FILE

	/*if (bAiming && bContinuousStreamOfFire) {
		
		//float pitchNum = FMath::RInterpTo(FirstPersonCameraComponent->RelativeRotation, recoilRotation, GetWorld()->GetDeltaSeconds(), 10).Pitch;

		FP_ADSRotationPoint->SetRelativeRotation(FMath::RInterpTo(FP_ADSRotationPoint->RelativeRotation, recoilRotation, DeltaTime, 40.f));

		AddControllerPitchInputDespiteRoll(-0.1f);
		AddControllerYawInputDespiteRoll(0.02f);

		if (FMath::IsNearlyEqual(FP_ADSRotationPoint->RelativeRotation.Pitch, recoilRotation.Pitch, 0.01f)) {
			bContinuousStreamOfFire = false;
			bRecoveringFromRecoil = true;
			FP_ADSRotationPoint->SetRelativeRotation(recoilRotation);
		}

		
	}
	else if (bAiming && bRecoveringFromRecoil && FirstPersonCameraComponent->RelativeRotation != firstShotRotation)
	{
		if (FMath::IsNearlyEqual(FP_ADSRotationPoint->RelativeRotation.Yaw, 0.f, 0.01f)) {
			FP_ADSRotationPoint->SetRelativeRotation(FRotator::ZeroRotator);
		}
		else 
		{
			FP_ADSRotationPoint->SetRelativeRotation(FMath::RInterpTo(FP_ADSRotationPoint->RelativeRotation, FRotator::ZeroRotator, DeltaTime, 20.f));
		}

		if (FMath::IsNearlyEqual(FirstPersonCameraComponent->RelativeRotation.Pitch, firstShotRotation.Pitch, 0.1f)) {
			//FirstPersonCameraComponent->SetRelativeRotation(firstShotRotation);
			bRecoveringFromRecoil = false;
			UE_LOG(LogTemp, Warning, TEXT("LOOOOOOOOOOOOOOOOOOOOOOOOL"));
		}
		else
		{
			FRotator gradualReset = FMath::RInterpTo(FirstPersonCameraComponent->RelativeRotation, firstShotRotation, DeltaTime, 4.f);
			//AddControllerPitchInput(0.01f);

			AddControllerPitchInput((gradualReset - FirstPersonCameraComponent->RelativeRotation).Pitch / -2.5f);
			AddControllerYawInput((gradualReset - FirstPersonCameraComponent->RelativeRotation).Yaw / 1.75f);
		}
	}*/

	// Crouch code
	if (bCrouched)
	{
		if (crouchRepositionAlpha >= 1.f) {
			return;
			//UE_LOG(LogTemp, Warning, TEXT("RECOIL RECOVERY COMPLETE"));
			recoilRecoveryLerpAlpha = 0.f;
			crouchMovementSinceLastFrame = 0;
		}

		float crouchMovementThisFrame = FMath::Lerp(0.f, crouchCameraVerticalOffset, crouchRepositionAlpha) - crouchMovementSinceLastFrame;

		crouchRepositionAlpha += 0.1f;
		crouchMovementSinceLastFrame += crouchMovementThisFrame;
		FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 0.f, -crouchMovementThisFrame));
	}
	else
	{
		if (crouchRepositionAlpha <= 0.f) {
			return;
			//UE_LOG(LogTemp, Warning, TEXT("RECOIL RECOVERY COMPLETE"));
			recoilRecoveryLerpAlpha = 1.f;
			crouchMovementSinceLastFrame = 0;
		}

		float crouchMovementThisFrame = FMath::Lerp(0.f, crouchCameraVerticalOffset, crouchRepositionAlpha) - crouchMovementSinceLastFrame;

		crouchRepositionAlpha -= 0.1f;
		crouchMovementSinceLastFrame += crouchMovementThisFrame;
		FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 0.f, -crouchMovementThisFrame));
	}
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
		float finalPitch = -1.f * yaw * FMath::Sin(roll);
		float finalYaw = yaw * FMath::Cos(roll);
	//}

	AddControllerPitchInput(finalPitch);
	AddControllerYawInput(finalYaw);
}

/** Gun rigger pulled. */
void AWarFantasyCharacter::OnFireDown()
{
	bTriggerCurrentlyPulled = true;
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

	// Get the animation object for the arms mesh
	UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();

	// try and play a firing animation if specified
	if (!bAiming && FireAnimation != NULL)
	{
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
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

		bContinuousStreamOfFire = true;
		rotationSinceLastShot = FRotator::ZeroRotator;
	}

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

			//TODO delay code (I'm starting to think all animations should be done through blueprints)
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

	FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 39.f, 0.f));
	AddControllerRollInput(leanRotationAmount);
}

/** Enable player lean right */
void AWarFantasyCharacter::LeanLeft()
{
	if (bLeaningRight || bSprinting) return;
	bLeaningLeft = true;

	FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, -39.f, 0.f));
	AddControllerRollInput(-leanRotationAmount);
}

/** Disable player lean */
void AWarFantasyCharacter::StandStraight()
{
	if (bLeaningRight)
	{
		AddControllerRollInput(-leanRotationAmount);
		FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, -39.f, 0.f));
	}
	else if (bLeaningLeft) 
	{
		AddControllerRollInput(leanRotationAmount);
		FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 39.f, 0.f));
	}

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

