#pragma once

#include "CoreMinimal.h"
#include "XPBDClothTypes.h"

class FRDGBuilder;

void AddXPBDClothPasses(
    FRDGBuilder& GraphBuilder,
    const FClothSimParams& Sim,
    FRDGBufferRef PositionBuffer,
    FRDGBufferRef VelocityBuffer,
    FRDGBufferRef PredictedBuffer,
    FRDGBufferRef ConstraintBuffer,
    FRDGBufferRef LambdaBuffer);
