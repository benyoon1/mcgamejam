#include "Aircraft_Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "AeroSurfaceComponent.h"
#include "AircraftPhysicsComponent.h"
#include "Engine/World.h"

AAircraft_Pawn::AAircraft_Pawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root mesh with physics simulation
	AircraftMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AircraftMesh"));
	SetRootComponent(AircraftMesh);
	AircraftMesh->SetSimulatePhysics(true);
	AircraftMesh->SetEnableGravity(true);
	AircraftMesh->SetLinearDamping(0.f);
	AircraftMesh->SetAngularDamping(0.f);

	// Physics component
	AircraftPhysics = CreateDefaultSubobject<UAircraftPhysicsComponent>(TEXT("AircraftPhysics"));

	// Spring arm for camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(AircraftMesh);
	SpringArm->TargetArmLength = 600.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 5.f;
	SpringArm->bDoCollisionTest = false;

	// Follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void AAircraft_Pawn::BeginPlay()
{
	Super::BeginPlay();

	this->GetComponents<UAeroSurfaceComponent>(SurfaceComponents);
}

void AAircraft_Pawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update flap/aileron/rudder deflections
	UpdateControlSurfaces();

	// Update throttle to physics system
	if (AircraftPhysics)
	{
		AircraftPhysics->SetThrustPercent(ThrottleInput);
	}
}

void AAircraft_Pawn::UpdateControlSurfaces()
{
	if (!AircraftPhysics) return;

	for (UAeroSurfaceComponent* Surface : SurfaceComponents)
	{
		if (!Surface || !Surface->bIsControlSurface) continue;

		switch (Surface->InputType)
		{
		case EControlInputType::Pitch:
			Surface->SetFlapAngle(PitchInput * Surface->InputMultiplier);
			break;
		case EControlInputType::Roll:
			Surface->SetFlapAngle(RollInput * Surface->InputMultiplier);
			break;
		case EControlInputType::Yaw:
			Surface->SetFlapAngle(YawInput * Surface->InputMultiplier);
			break;
		case EControlInputType::Flap:
			Surface->SetFlapAngle(FlapInput * Surface->InputMultiplier);
			break;
		}
	}
}
