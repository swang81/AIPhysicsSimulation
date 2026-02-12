#include "XPBDClothPasses.h"

#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"

class FClothPredictCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FClothPredictCS);
    SHADER_USE_PARAMETER_STRUCT(FClothPredictCS, FGlobalShader);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(float, DeltaTime)
        SHADER_PARAMETER(float, Damping)
        SHADER_PARAMETER(float, GravityZ)
        SHADER_PARAMETER(uint32, NumParticles)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float4>, InPosition)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float4>, InVelocity)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float4>, OutPredicted)
    END_SHADER_PARAMETER_STRUCT()
};

class FClothSolveDistanceCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FClothSolveDistanceCS);
    SHADER_USE_PARAMETER_STRUCT(FClothSolveDistanceCS, FGlobalShader);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(float, DeltaTime)
        SHADER_PARAMETER(float, Compliance)
        SHADER_PARAMETER(uint32, NumConstraints)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float4>, Predicted)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float4>, Constraints)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float>, Lambda)
    END_SHADER_PARAMETER_STRUCT()
};

class FClothUpdateVelocityCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FClothUpdateVelocityCS);
    SHADER_USE_PARAMETER_STRUCT(FClothUpdateVelocityCS, FGlobalShader);

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(float, DeltaTime)
        SHADER_PARAMETER(uint32, NumParticles)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float4>, Position)
        SHADER_PARAMETER_RDG_BUFFER_UAV(RWStructuredBuffer<float4>, Velocity)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float4>, Predicted)
    END_SHADER_PARAMETER_STRUCT()
};

IMPLEMENT_GLOBAL_SHADER(FClothPredictCS, "/XPBDCloth/Private/XPBDCloth.usf", "PredictPositionsCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FClothSolveDistanceCS, "/XPBDCloth/Private/XPBDCloth.usf", "SolveDistanceConstraintsCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FClothUpdateVelocityCS, "/XPBDCloth/Private/XPBDCloth.usf", "UpdateVelocityCS", SF_Compute);

void AddXPBDClothPasses(
    FRDGBuilder& GraphBuilder,
    const FClothSimParams& Sim,
    FRDGBufferRef PositionBuffer,
    FRDGBufferRef VelocityBuffer,
    FRDGBufferRef PredictedBuffer,
    FRDGBufferRef ConstraintBuffer,
    FRDGBufferRef LambdaBuffer)
{
    {
        FClothPredictCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FClothPredictCS::FParameters>();
        PassParameters->DeltaTime = Sim.DeltaTime;
        PassParameters->Damping = Sim.Damping;
        PassParameters->GravityZ = Sim.GravityZ;
        PassParameters->NumParticles = Sim.NumParticles;
        PassParameters->InPosition = GraphBuilder.CreateSRV(PositionBuffer);
        PassParameters->InVelocity = GraphBuilder.CreateSRV(VelocityBuffer);
        PassParameters->OutPredicted = GraphBuilder.CreateUAV(PredictedBuffer);

        const TShaderMapRef<FClothPredictCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("ClothXPBD.Predict"),
            Shader,
            PassParameters,
            FIntVector(FMath::DivideAndRoundUp<int32>((int32)Sim.NumParticles, 64), 1, 1));
    }

    for (uint32 Iter = 0; Iter < Sim.IterationCount; ++Iter)
    {
        FClothSolveDistanceCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FClothSolveDistanceCS::FParameters>();
        PassParameters->DeltaTime = Sim.DeltaTime;
        PassParameters->Compliance = Sim.Compliance;
        PassParameters->NumConstraints = Sim.NumConstraints;
        PassParameters->Predicted = GraphBuilder.CreateUAV(PredictedBuffer);
        PassParameters->Constraints = GraphBuilder.CreateSRV(ConstraintBuffer);
        PassParameters->Lambda = GraphBuilder.CreateUAV(LambdaBuffer);

        const TShaderMapRef<FClothSolveDistanceCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("ClothXPBD.SolveDistance.Iter%d", Iter),
            Shader,
            PassParameters,
            FIntVector(FMath::DivideAndRoundUp<int32>((int32)Sim.NumConstraints, 64), 1, 1));
    }

    {
        FClothUpdateVelocityCS::FParameters* PassParameters = GraphBuilder.AllocParameters<FClothUpdateVelocityCS::FParameters>();
        PassParameters->DeltaTime = Sim.DeltaTime;
        PassParameters->NumParticles = Sim.NumParticles;
        PassParameters->Position = GraphBuilder.CreateUAV(PositionBuffer);
        PassParameters->Velocity = GraphBuilder.CreateUAV(VelocityBuffer);
        PassParameters->Predicted = GraphBuilder.CreateSRV(PredictedBuffer);

        const TShaderMapRef<FClothUpdateVelocityCS> Shader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
        FComputeShaderUtils::AddPass(
            GraphBuilder,
            RDG_EVENT_NAME("ClothXPBD.UpdateVelocity"),
            Shader,
            PassParameters,
            FIntVector(FMath::DivideAndRoundUp<int32>((int32)Sim.NumParticles, 64), 1, 1));
    }
}
