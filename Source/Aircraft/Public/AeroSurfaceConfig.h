#pragma once

#include "CoreMinimal.h"
#include "AeroSurfaceConfig.generated.h"

USTRUCT(BlueprintType)
struct FAeroSurfaceConfig
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float LiftSlope = 6.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float SkinFriction = 0.02f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float ZeroLiftAoA = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float StallAngleHigh = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float StallAngleLow = -15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float Chord = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float FlapFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	float Span = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface")
	bool bAutoAspectRatio = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aero Surface", meta = (EditCondition = "!bAutoAspectRatio", EditConditionHides))
	float AspectRatio = 2.0f;

	void Validate()
	{
		FlapFraction = FMath::Clamp(FlapFraction, 0.0f, 0.4f);
		StallAngleHigh = FMath::Max(StallAngleHigh, 0.0f);
		StallAngleLow = FMath::Min(StallAngleLow, 0.0f);
		Chord = FMath::Max(Chord, 1e-3f);

		if (bAutoAspectRatio)
		{
			AspectRatio = Span / Chord;
		}
	}
};