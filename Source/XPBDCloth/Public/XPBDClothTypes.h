#pragma once

#include "CoreMinimal.h"

struct FClothSimParams
{
    float DeltaTime = 1.0f / 60.0f;
    float Damping = 0.02f;
    float GravityZ = -980.0f;
    float Compliance = 1.0e-6f;
    uint32 NumParticles = 0;
    uint32 NumConstraints = 0;
    uint32 IterationCount = 8;
};

// x=i, y=j, z=restLength, w=unused
using FDistanceConstraintData = FVector4f;
