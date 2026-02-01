#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AircraftPhysicsComponent.h"
#include "AeroSurfaceComponent.h"
#include "Aircraft_Pawn.generated.h"

UCLASS()
class AIRCRAFT_API AAircraft_Pawn : public APawn
{
	GENERATED_BODY()

public:
	AAircraft_Pawn();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aircraft")
	USkeletalMeshComponent* AircraftMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aircraft")
	UAircraftPhysicsComponent* AircraftPhysics;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Control")
	float ThrottleInput = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Control")
	float PitchInput = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Control")
	float RollInput = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Control")
	float YawInput = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aircraft Control")
	float FlapInput = 0.f;

private:
	TArray<UAeroSurfaceComponent*> SurfaceComponents;

	void UpdateControlSurfaces();
};