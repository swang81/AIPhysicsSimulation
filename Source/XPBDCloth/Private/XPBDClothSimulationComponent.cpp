#include "XPBDClothSimulationComponent.h"

#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RHICommandList.h"
#include "XPBDClothPasses.h"

UXPBDClothSimulationComponent::UXPBDClothSimulationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UXPBDClothSimulationComponent::BeginPlay()
{
    Super::BeginPlay();
    BuildInitialData();
}

void UXPBDClothSimulationComponent::BuildInitialData()
{
    Positions.Reset();
    Velocities.Reset();
    Constraints.Reset();

    const int32 NumParticles = GridWidth * GridHeight;
    Positions.Reserve(NumParticles);
    Velocities.Reserve(NumParticles);

    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            const float InvMass = (Y == 0 && (X % 6 == 0)) ? 0.0f : 1.0f;
            Positions.Emplace(X * ParticleSpacing, 0.0f, -Y * ParticleSpacing, InvMass);
            Velocities.Emplace(0, 0, 0, 0);

            if (X + 1 < GridWidth)
            {
                Constraints.Emplace((float)(Y * GridWidth + X), (float)(Y * GridWidth + X + 1), ParticleSpacing, 0.0f);
            }
            if (Y + 1 < GridHeight)
            {
                Constraints.Emplace((float)(Y * GridWidth + X), (float)((Y + 1) * GridWidth + X), ParticleSpacing, 0.0f);
            }
        }
    }
}

void UXPBDClothSimulationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (Positions.IsEmpty() || Constraints.IsEmpty())
    {
        return;
    }

    const FClothSimParams SimParams = {
        DeltaTime,
        0.02f,
        -980.0f,
        1.0e-6f,
        (uint32)Positions.Num(),
        (uint32)Constraints.Num(),
        IterationCount
    };

    TArray<FVector4f> LocalPositions = Positions;
    TArray<FVector4f> LocalVelocities = Velocities;
    TArray<FDistanceConstraintData> LocalConstraints = Constraints;

    ENQUEUE_RENDER_COMMAND(XPBDClothDispatch)(
        [SimParams, LocalPositions = MoveTemp(LocalPositions), LocalVelocities = MoveTemp(LocalVelocities), LocalConstraints = MoveTemp(LocalConstraints)](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            FRDGBufferRef PositionBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("XPBD.Position"), LocalPositions);
            FRDGBufferRef VelocityBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("XPBD.Velocity"), LocalVelocities);
            FRDGBufferRef ConstraintBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("XPBD.Constraint"), LocalConstraints);

            FRDGBufferRef PredictedBuffer = GraphBuilder.CreateBuffer(
                FRDGBufferDesc::CreateStructuredDesc(sizeof(FVector4f), LocalPositions.Num()),
                TEXT("XPBD.Predicted"));

            TArray<float> InitialLambda;
            InitialLambda.Init(0.0f, LocalConstraints.Num());
            FRDGBufferRef LambdaBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("XPBD.Lambda"), InitialLambda);

            AddXPBDClothPasses(
                GraphBuilder,
                SimParams,
                PositionBuffer,
                VelocityBuffer,
                PredictedBuffer,
                ConstraintBuffer,
                LambdaBuffer);

            GraphBuilder.Execute();
        });
}
