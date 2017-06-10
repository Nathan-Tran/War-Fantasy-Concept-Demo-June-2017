// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "WarFantasy.h"
#include "WarFantasyCharacter.h"
#include "WarFantasyProjectile.h" //TODO remove
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

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(0.f, -90.f, 0.f);  //TODO shrink and reposition gun
	Mesh1P->RelativeLocation = FVector(0.f, 0.f, -16.5f);
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

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWarFantasyCharacter::OnFire);
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
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWarFantasyCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWarFantasyCharacter::LookUpAtRate);
}

void AWarFantasyCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation() + FRotator(30.f, 90.f, 0.f);
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			
			// I don't think we need this redundency
			//const FVector SpawnLocation = ((FP_WeaponBreachLocation != nullptr) ? FP_WeaponBreachLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);
			const FVector SpawnLocation = FP_WeaponBreachLocation->GetComponentLocation();

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			World->SpawnActor<AWarFantasyProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
			//TODO perhaps play both gun and hand animations here
		}
	}
}

void AWarFantasyCharacter::StartSprint()
{
	bSprinting = true;

	GetCharacterMovement()->MaxWalkSpeed = sprintSpeed;

}

void AWarFantasyCharacter::StopSprint()
{
	bSprinting = false;

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed;
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

	GetCharacterMovement()->MaxWalkSpeed = ADSWalkSpeed;

}

void AWarFantasyCharacter::OnLookAwayFromSights()
{
	bAiming = false;

	GetCharacterMovement()->MaxWalkSpeed = walkSpeed; //TODO this logic ain't gon play
}

/** Cease ADS. */
void AWarFantasyCharacter::OnCrouchDown()
{
	UE_LOG(LogTemp, Warning, TEXT("CROUCH PRESSED"));
	bCrouched = true;

	//FirstPersonCameraComponent->RelativeLocation = FVector(0.f, 0.f, 0.f); // Position the camera

	Crouch();

}

/** Cease ADS. */
void AWarFantasyCharacter::OnStandUp()
{
	bCrouched = false; //TODO possibly unused

	UnCrouch();
}

/** Enable player lean right */
void AWarFantasyCharacter::LeanRight()
{
	bLeaningRight = true;

	FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, 39.f, 0.f));
	AddControllerRollInput(30.f);
}

/** Enable player lean right */
void AWarFantasyCharacter::LeanLeft()
{
	bLeaningLeft = true;

	FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, -39.f, 0.f));
	AddControllerRollInput(-30.f);
}

/** Disable player lean */
void AWarFantasyCharacter::StandStraight()
{
	if (bLeaningRight)
	{
		AddControllerRollInput(-30.f);
		FirstPersonCameraComponent->AddRelativeLocation(FVector(0.f, -39.f, 0.f));
	}
	else if (bLeaningLeft) 
	{
		AddControllerRollInput(30.f);
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
//TODO fix aiming when leaning
//TODO lock animations when reloading
//TODO re-implement gun firing and mussle flash animation
//TODO factor gun rotation into shell casing ejection angle
//TODO fix bug where shell casings collide with gun and cause a phsyics impact
//TODO remove dependance on blueprint to change camera/mesh scale

