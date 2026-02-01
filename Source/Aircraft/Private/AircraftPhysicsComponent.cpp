#include "AircraftPhysicsComponent.h"
#include "GameFramework//Actor.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UAircraftPhysicsComponent::UAircraftPhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAircraftPhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (Mesh)
	{
		PhysicsComponent = Mesh;
	}
	GetOwner()->GetComponents<UAeroSurfaceComponent>(AeroSurfaces);

	for (UAeroSurfaceComponent* Surface : AeroSurfaces) {
		if (!Surface) continue;
		Surface->bEnableDebug = bEnableDebug;
	}
}

void UAircraftPhysicsComponent::SetThrustPercent(float Percent)
{
	ThrustPercent = Percent;
}

void UAircraftPhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!PhysicsComponent) return;

	const FVector LinearVelocity = PhysicsComponent->GetPhysicsLinearVelocity();
	const FVector AngularVelocity = PhysicsComponent->GetPhysicsAngularVelocityInRadians();
	const FVector CenterOfMass = PhysicsComponent->GetCenterOfMass();

	// Calculate aerodynamic forces
	FBiVector3 ForceAndTorqueThisFrame = CalculateAerodynamicForces(LinearVelocity, AngularVelocity, FVector::ZeroVector, AirDensity, CenterOfMass);

	CurrentForceAndTorque = ForceAndTorqueThisFrame;

	// Apply forces
	PhysicsComponent->AddForce(CurrentForceAndTorque.P / 100.f);
	PhysicsComponent->AddTorqueInRadians(CurrentForceAndTorque.Q / 100.f);

	// Apply thrust
	PhysicsComponent->AddForce(PhysicsComponent->GetForwardVector() * Thrust * ThrustPercent);
}

FBiVector3 UAircraftPhysicsComponent::CalculateAerodynamicForces(const FVector& Velocity, const FVector& AngularVelocity, const FVector& Wind, float Density, const FVector& CenterOfMass)
{
	FBiVector3 ForceAndTorque;
	for (UAeroSurfaceComponent* Surface : AeroSurfaces)
	{
		FVector relativePosition = Surface->GetComponentLocation() - CenterOfMass;
		FVector EffectiveVelocity = -Velocity + Wind - FVector::CrossProduct(AngularVelocity, relativePosition);

		ForceAndTorque += Surface->CalculateForces(EffectiveVelocity, Density, relativePosition);
	}

	return ForceAndTorque;
}