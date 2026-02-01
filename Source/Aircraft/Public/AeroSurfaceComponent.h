#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "AeroSurfaceConfig.h"
#include "BiVector3.h"
#include "AeroSurfaceComponent.generated.h"

UENUM(BlueprintType)
enum class EControlInputType : uint8
{
	Pitch UMETA(DisplayName = "Pitch"),
	Yaw UMETA(DisplayName = "Yaw"),
	Roll UMETA(DisplayName = "Roll"),
	Flap UMETA(DisplayName = "Flap")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AIRCRAFT_API UAeroSurfaceComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAeroSurfaceComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aero Surface")
	FAeroSurfaceConfig Config;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Aero Surface")
	bool bIsControlSurface = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface", meta = (EditCondition = "bIsControlSurface", EditConditionHides))
	EControlInputType InputType = EControlInputType::Pitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface", meta = (EditCondition = "bIsControlSurface", EditConditionHides))
	float InputMultiplier = 1.0f;

#if WITH_EDITOR
	UStaticMeshComponent* SurfaceMesh;

	UStaticMeshComponent* FlapMesh;

	USceneComponent* FlapHinge;

	UArrowComponent* UpArrow;

	UMaterialInstanceDynamic* SurfaceDynMat;

	UMaterialInstanceDynamic* FlapDynMat;
#endif

	bool bEnableDebug = false;

	float FlapAngle = 0.0f;

	void SetFlapAngle(float AngleRadians);

	FBiVector3 CalculateForces(const FVector& WorldAirVelocity, float AirDensity, const FVector& RelativePosition);

public:
	FVector CalculateCoefficients(float AoA, float CorrectedLiftSlope, float ZeroLiftAoA, float StallHigh, float StallLow);
	FVector CalculateCoefficientsAtLowAoA(float AoA, float CorrectedLiftSlope, float ZeroLiftAoA);
	FVector CalculateCoefficientsAtStall(float AoA, float CorrectedLiftSlope, float ZeroLiftAoA, float StallHigh, float StallLow);

	float TorqueCoefficientProportion(float EffectiveAngle);
	float FrictionAt90Degrees(float FlapAngle);
	float FlapEffectivenessCorrection(float FlapAngle);
	float LiftCoefficientMaxFraction(float FlapFraction);
};