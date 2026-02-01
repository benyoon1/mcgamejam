#include "AeroSurfaceComponent.h"
#include "DrawDebugHelpers.h"

UAeroSurfaceComponent::UAeroSurfaceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

#if WITH_EDITOR
	// Create a plane mesh for visualization
	SurfaceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SurfaceMesh"));
	FlapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlapMesh"));
	UpArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("UpArrow"));
	FlapHinge = CreateDefaultSubobject<USceneComponent>(TEXT("FlapHinge"));

	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UMaterialInstance* VisualMat = LoadObject<UMaterialInstance>(nullptr, TEXT("/Game/Visualizer/M_SurfaceVisual_Inst.M_SurfaceVisual_Inst"));

	if (PlaneMesh)
	{
		if (SurfaceMesh)
		{
			SurfaceMesh->SetStaticMesh(PlaneMesh);
			SurfaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SurfaceMesh->SetMobility(EComponentMobility::Movable);
			SurfaceMesh->bHiddenInGame = false;

			if (VisualMat)
			{
				SurfaceDynMat = UMaterialInstanceDynamic::Create(VisualMat, this);
				SurfaceDynMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0, 0, 0.5f));
				SurfaceMesh->SetMaterial(0, SurfaceDynMat);
			}
		}

		if (FlapMesh)
		{
			FlapMesh->SetStaticMesh(PlaneMesh);
			FlapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			FlapMesh->SetMobility(EComponentMobility::Movable);
			FlapMesh->bHiddenInGame = false;

			if (VisualMat)
			{
				FlapDynMat = UMaterialInstanceDynamic::Create(VisualMat, this);
				FlapDynMat->SetVectorParameterValue(TEXT("Color"), FLinearColor::Yellow);
				FlapMesh->SetMaterial(0, FlapDynMat);
			}
		}

		if (UpArrow)
		{
			UpArrow->ArrowColor = FColor::Cyan;
			UpArrow->ArrowSize = 0.5f;
			UpArrow->bHiddenInGame = false;
			UpArrow->SetRelativeScale3D(FVector(0.5f));
			UpArrow->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
		}
	}
#endif
}

void UAeroSurfaceComponent::OnRegister()
{
    Super::OnRegister();

#if WITH_EDITOR
    if (!SurfaceMesh || !FlapMesh)
        return;

	SurfaceMesh->SetupAttachment(this);
	FlapHinge->SetupAttachment(this);
	FlapMesh->SetupAttachment(FlapHinge);
	UpArrow->SetupAttachment(this);

    const float SurfaceChord = Config.Chord * (1.0f - Config.FlapFraction);
    const float FlapChord = Config.Chord * Config.FlapFraction;

    FVector ParentWorldScale = GetComponentTransform().GetScale3D();

    const float GapFraction = 0.01f; // gap as fraction of chord
	const float Gap = 0.5f * Config.Chord * GapFraction * 100.f;

	float SurfaceHalfLength = 0.5f * SurfaceChord / ParentWorldScale.Y * 100.f;
	float FlapHalfLength = 0.5f * FlapChord / ParentWorldScale.Y * 100.f;

    // Set SurfaceMesh
    FVector SurfaceDesiredScale(Config.Span / ParentWorldScale.X, SurfaceChord / ParentWorldScale.Y, 1.f / ParentWorldScale.Z);
    SurfaceMesh->SetRelativeScale3D(SurfaceDesiredScale);
    SurfaceMesh->SetRelativeLocation(FVector(0.f, -FlapHalfLength - Gap, 0.f));

    // Set Hinge
	FlapHinge->SetRelativeLocation(FVector(0.f, -FlapHalfLength + SurfaceHalfLength, 0.f));

	// Flap Mesh
	FlapMesh->SetRelativeScale3D(FVector(Config.Span / ParentWorldScale.X, FlapChord / ParentWorldScale.Y, 1.f / ParentWorldScale.Z));
	FlapMesh->SetRelativeLocation(FVector(0.f, FlapHalfLength, 0.f));
#endif
}


void UAeroSurfaceComponent::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if (SurfaceMesh)
		SurfaceMesh->SetVisibility(bEnableDebug);
	if (FlapMesh)
		FlapMesh->SetVisibility(bEnableDebug);
	if (UpArrow)
		UpArrow->SetVisibility(bIsControlSurface && bEnableDebug);
#endif
}

void UAeroSurfaceComponent::SetFlapAngle(float AngleRadians)
{
	FlapAngle = FMath::Clamp(AngleRadians, -FMath::DegreesToRadians(50.f), FMath::DegreesToRadians(50.f));

#if WITH_EDITOR
	if (bEnableDebug) {
		const FRotator FlapRotation = FRotator(0.f, 0.f, FMath::RadiansToDegrees(FlapAngle));
		FlapHinge->SetRelativeRotation(FlapRotation);
	}
#endif
}

FBiVector3 UAeroSurfaceComponent::CalculateForces(const FVector& WorldAirVelocity, float AirDensity, const FVector& RelativePosition)
{
	FBiVector3 forceAndTorque;
	Config.Validate();

	// Corrected lift slope (aspect ratio effect)
	const float CorrectedLiftSlope = Config.LiftSlope * Config.AspectRatio / (Config.AspectRatio + 2 * (Config.AspectRatio + 4) / (Config.AspectRatio + 2));

	// Flap influence
	const float Theta = FMath::Acos(2 * Config.FlapFraction - 1);
	const float FlapEffectiveness = 1 - (Theta - FMath::Sin(Theta)) / PI;
	const float DeltaLift = CorrectedLiftSlope * FlapEffectiveness * FlapEffectivenessCorrection(FlapAngle) * FlapAngle;

	const float ZeroLiftAoABase = FMath::DegreesToRadians(Config.ZeroLiftAoA);
	const float ZeroLiftAoA = ZeroLiftAoABase - DeltaLift / CorrectedLiftSlope;

	const float StallAngleHighBase = FMath::DegreesToRadians(Config.StallAngleHigh);
	const float StallAngleLowBase = FMath::DegreesToRadians(Config.StallAngleLow);

	const float ClMaxHigh = CorrectedLiftSlope * (StallAngleHighBase - ZeroLiftAoABase) + DeltaLift * LiftCoefficientMaxFraction(Config.FlapFraction);
	const float ClMaxLow = CorrectedLiftSlope * (StallAngleLowBase - ZeroLiftAoABase) + DeltaLift * LiftCoefficientMaxFraction(Config.FlapFraction);

	const float StallHigh = ZeroLiftAoA + ClMaxHigh / CorrectedLiftSlope;
	const float StallLow = ZeroLiftAoA + ClMaxLow / CorrectedLiftSlope;

	// Transform world air velocity into local space
	FVector LocalAirVelocity = GetComponentTransform().InverseTransformVectorNoScale(WorldAirVelocity);
	LocalAirVelocity = FVector(0.f, LocalAirVelocity.Y, LocalAirVelocity.Z);

	const FVector DragDir = GetComponentTransform().TransformVectorNoScale(LocalAirVelocity.GetSafeNormal());

	const FVector LiftDir = FVector::CrossProduct(DragDir, -GetForwardVector());

	const float Area = Config.Chord * Config.Span;
	const float DynamicPressure = 0.5f * AirDensity * LocalAirVelocity.SizeSquared();
	const float AoA = FMath::Atan2(LocalAirVelocity.Z, LocalAirVelocity.Y);

	const FVector Coeffs = CalculateCoefficients(AoA, CorrectedLiftSlope, ZeroLiftAoA, StallHigh, StallLow);

	const FVector Lift = LiftDir * Coeffs.X * DynamicPressure * Area;
	const FVector Drag = DragDir * Coeffs.Y * DynamicPressure * Area;
	const FVector Torque = GetForwardVector() * Coeffs.Z * DynamicPressure * Area * Config.Chord;

	forceAndTorque.P += Lift + Drag;
	forceAndTorque.Q += FVector::CrossProduct(RelativePosition, forceAndTorque.P);
	forceAndTorque.Q += Torque;

	if (bEnableDebug) {
		const float VisualScaleF = 0.00003f;
		const FVector WorldCoM = GetComponentTransform().TransformPosition(FVector::ZeroVector);
		DrawDebugDirectionalArrow(GetWorld(), WorldCoM, WorldCoM + Lift * VisualScaleF, 20.f, FColor::Green, false, 0.f, 0, 2.f);
		DrawDebugDirectionalArrow(GetWorld(), WorldCoM, WorldCoM + Drag * VisualScaleF, 20.f, FColor::Red, false, 0.f, 0, 2.f);
		DrawDebugDirectionalArrow(GetWorld(), WorldCoM, WorldCoM + Torque * VisualScaleF, 20.f, FColor::Yellow, false, 0.f, 0, 2.f);
	}

	return forceAndTorque;
}

FVector UAeroSurfaceComponent::CalculateCoefficients(float AoA, float CLS, float ZeroLiftAoA, float StallHigh, float StallLow)
{
	FVector Coeffs;

	const float PaddingHigh = FMath::DegreesToRadians(FMath::Lerp(15.f, 5.f, (FMath::RadiansToDegrees(FlapAngle) + 50) / 100.f));
	const float PaddingLow = FMath::DegreesToRadians(FMath::Lerp(15.f, 5.f, (-FMath::RadiansToDegrees(FlapAngle) + 50) / 100.f));
	const float PaddedHigh = StallHigh + PaddingHigh;
	const float PaddedLow = StallLow - PaddingLow;

	if (AoA < StallHigh && AoA > StallLow) {
		Coeffs = CalculateCoefficientsAtLowAoA(AoA, CLS, ZeroLiftAoA);
	}
	else
	{
		if (AoA > PaddedHigh || AoA < PaddedLow)
		{
			Coeffs = CalculateCoefficientsAtStall(AoA, CLS, ZeroLiftAoA, StallHigh, StallLow);
		}
		else
		{
			FVector CoeffsLow, CoeffsStall;
			float LerpT = 0;
			if (AoA > StallHigh)
			{
				CoeffsLow = CalculateCoefficientsAtLowAoA(StallHigh, CLS, ZeroLiftAoA);
				CoeffsStall = CalculateCoefficientsAtStall(PaddedHigh, CLS, ZeroLiftAoA, StallHigh, StallLow);
				LerpT = (AoA - StallHigh) / (PaddedHigh - StallHigh);
			}
			else
			{
				CoeffsLow = CalculateCoefficientsAtLowAoA(StallLow, CLS, ZeroLiftAoA);
				CoeffsStall = CalculateCoefficientsAtStall(PaddedLow, CLS, ZeroLiftAoA, StallHigh, StallLow);
				LerpT = (AoA - StallLow) / (PaddedLow - StallLow);
			}
			Coeffs = FMath::Lerp(CoeffsLow, CoeffsStall, LerpT);
		}
	}

#if WITH_EDITOR
	if (bEnableDebug) {
		const bool IsAtStall = !(AoA < StallHigh && AoA > StallLow);
		SurfaceDynMat->SetVectorParameterValue(TEXT("Color"), IsAtStall ? FLinearColor(1.0f, 0, 0) : FLinearColor(0, 0, 0.5f));
		FlapDynMat->SetVectorParameterValue(TEXT("Color"), IsAtStall ? FLinearColor(1.0f, 0, 0) : FLinearColor::Yellow);
	}
#endif

	return Coeffs;
}

FVector UAeroSurfaceComponent::CalculateCoefficientsAtLowAoA(float AoA, float CLS, float ZeroLiftAoA)
{
	const float LiftCoeff = CLS * (AoA - ZeroLiftAoA);
	const float InducedAngle = LiftCoeff / (PI * Config.AspectRatio);
	const float EffAngle = AoA - ZeroLiftAoA - InducedAngle;

	const float Tangential = Config.SkinFriction * FMath::Cos(EffAngle);
	const float Normal = (LiftCoeff + FMath::Sin(EffAngle) * Tangential) / FMath::Cos(EffAngle);
	const float DragCoeff = Normal * FMath::Sin(EffAngle) + Tangential * FMath::Cos(EffAngle);
	const float TorqueCoeff = -Normal * TorqueCoefficientProportion(EffAngle);

	return FVector(LiftCoeff, DragCoeff, TorqueCoeff);
}

FVector UAeroSurfaceComponent::CalculateCoefficientsAtStall(float AoA, float CLS, float ZeroLiftAoA, float StallHigh, float StallLow)
{
	const float LiftLowAoA = (AoA > StallHigh)
		? CLS * (StallHigh - ZeroLiftAoA)
		: CLS * (StallLow - ZeroLiftAoA);

	float InducedAngle = LiftLowAoA / (PI * Config.AspectRatio);

	float LerpT = 0;

	if (AoA > StallHigh)
		LerpT = (HALF_PI - FMath::Clamp(AoA, -HALF_PI, HALF_PI)) / (HALF_PI - StallHigh);
	else
		LerpT = (-HALF_PI - FMath::Clamp(AoA, -HALF_PI, HALF_PI)) / (-HALF_PI - StallLow);

	InducedAngle = FMath::Lerp(0.f, InducedAngle, FMath::Clamp(LerpT, 0, 1));
	const float EffAngle = AoA - ZeroLiftAoA - InducedAngle;

	const float Normal = FrictionAt90Degrees(FlapAngle) * FMath::Sin(EffAngle) *
		(1.f / (0.56f + 0.44f * FMath::Abs(FMath::Sin(EffAngle))) -
			0.41f * (1.f - FMath::Exp(-17.f / Config.AspectRatio)));
	const float Tangential = 0.5f * Config.SkinFriction * FMath::Cos(EffAngle);

	const float LiftCoeff = Normal * FMath::Cos(EffAngle) - Tangential * FMath::Sin(EffAngle);
	const float DragCoeff = Normal * FMath::Sin(EffAngle) + Tangential * FMath::Cos(EffAngle);
	const float TorqueCoeff = -Normal * TorqueCoefficientProportion(EffAngle);

	return FVector(LiftCoeff, DragCoeff, TorqueCoeff);
}

float UAeroSurfaceComponent::TorqueCoefficientProportion(float EffAngle)
{
	return 0.25f - 0.175f * (1 - 2 * FMath::Abs(EffAngle) / PI);
}

float UAeroSurfaceComponent::FrictionAt90Degrees(float InFlapAngle)
{
	return 1.98f - 0.0426f * InFlapAngle * InFlapAngle + 0.21f * InFlapAngle;
}

float UAeroSurfaceComponent::FlapEffectivenessCorrection(float InFlapAngle)
{
	return FMath::Lerp(0.8f, 0.4f, (FMath::RadiansToDegrees(FMath::Abs(InFlapAngle)) - 10) / 50);
}

float UAeroSurfaceComponent::LiftCoefficientMaxFraction(float FlapFraction)
{
	return FMath::Clamp(1 - 0.5f * (FlapFraction - 0.1f) / 0.3f, 0.f, 1.f);
}