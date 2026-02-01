#pragma once

#include "CoreMinimal.h"
#include "BiVector3.generated.h"

USTRUCT(BlueprintType)
struct FBiVector3
{
	GENERATED_BODY()

public:
	// Force vector
	UPROPERTY(EDITAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	FVector P = FVector::ZeroVector;

	// Torque vector
	UPROPERTY(EDITAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	FVector Q = FVector::ZeroVector;

	FBiVector3() {}

	FBiVector3(const FVector& InForce, const FVector& InTorque)
		: P(InForce), Q(InTorque)
	{}

	FORCEINLINE FBiVector3 operator+(const FBiVector3& Other) const
	{
		return FBiVector3(P + Other.P, Q + Other.Q);
	}

	friend FBiVector3 operator*(float Scalar, const FBiVector3& A)
	{
		return FBiVector3(Scalar * A.P, Scalar * A.Q);
	}

	friend FBiVector3 operator*(const FBiVector3& A, float Scalar)
	{
		return Scalar * A;
	}

	FORCEINLINE FBiVector3& operator+=(const FBiVector3& Other)
	{
		P += Other.P;
		Q += Other.Q;
		return *this;
	}
};