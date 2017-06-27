// Fill out your copyright notice in the Description page of Project Settings.

#include "WarFantasy.h"
#include "PortalBaseClass.h"


// Sets default values
APortalBaseClass::APortalBaseClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SetRootComponent(PortalA);
	PortalA = CreateDefaultSubobject<USceneComponent>(TEXT("Portal A Location"));

	PortalDoorway = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Portal Doorway"));
	PortalDoorway->SetupAttachment(PortalA);
	PortalDoorway->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); //100 is the portal height/2. This should be refactored as a variable
	PortalDoorway->SetRelativeScale3D(FVector(2.f, 0.9f, 1.f));
	PortalDoorway->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	PortalDoorway->SetMobility(EComponentMobility::Movable);
	PortalDoorway->SetSimulatePhysics(false);
	PortalDoorway->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PortalDoorway->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObj(TEXT("/Game/PortalTech/PortalWall_Mesh"));
	if (MeshObj.Object)
		PortalDoorway->SetStaticMesh(MeshObj.Object);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PortalMat(TEXT("/Game/PortalTech/Portal1_RT_Mat"));
	if (PortalMat.Object)
		PortalDoorway->SetMaterial(0, PortalMat.Object);

	PortalCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Portal Capture"));
	//PortalCapture->SetupAttachment(PortalA);
	PortalCapture->FOVAngle = 90.f;

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTrgt(TEXT("/Game/PortalTech/Portal1_RT"));
	if (RenderTrgt.Object)
		PortalCapture->TextureTarget = RenderTrgt.Object;

}

// Called every frame
/*void APortalBaseClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

// Called when the game starts or when spawned
void APortalBaseClass::BeginPlay()
{
	Super::BeginPlay();

	thePlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	SetTickGroup(ETickingGroup::TG_PostUpdateWork); //TODO possible cause of white flash
	//SetTickGroup(ETickingGroup::TG_PrePhysics); //TODO possible cause of white flash
	//PortalCapture->MaxViewDistanceOverride = 25.f; //TODO possible cause of white flash
	PortalDoorway->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PortalDoorway->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// Create a reference to the player's camera
	UWorld* const pWorld = GetWorld();
	if (pWorld)
	{
		playerCamera = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(pWorld, 0));
	}

	PositionClippingPlane();
}

// Reposition the portal's clipping plane
void APortalBaseClass::PositionClippingPlane()
{
	FVector newRotationVector = PortalB->GetComponentRotation().Vector(); // Yes, I know it's bad naming convention
	PortalCapture->bEnableClipPlane = true;
	PortalCapture->ClipPlaneNormal = newRotationVector;
	PortalCapture->ClipPlaneBase = (newRotationVector * -1.f) + PortalB->GetComponentLocation();
}

