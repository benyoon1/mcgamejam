#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "AeroSurfaceComponent.h"
#include "BiVector3.h"
#include "AircraftPhysicsComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AIRCRAFT_API UAircraftPhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAircraftPhysicsComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Physics")
	float Thrust = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Physics")
	TArray<UAeroSurfaceComponent*> AeroSurfaces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Physics")
	float AirDensity = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Physics")
	bool bEnableDebug = false;

	void SetThrustPercent(float Percent);

public:
	float ThrustPercent = 0.f;
	FBiVector3 CurrentForceAndTorque;

	UPrimitiveComponent* PhysicsComponent = nullptr;

	FBiVector3 CalculateAerodynamicForces(const FVector& Velocity, const FVector& AngularVelocity, const FVector& Wind, float AirDensity, const FVector& CenterOfMass);
};