#pragma once

#include "Components/ActorComponent.h"
#include "XPBDClothTypes.h"
#include "XPBDClothSimulationComponent.generated.h"

UCLASS(ClassGroup=(Simulation), meta=(BlueprintSpawnableComponent))
class XPBDCLOTH_API UXPBDClothSimulationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UXPBDClothSimulationComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, Category="XPBD")
    int32 GridWidth = 32;

    UPROPERTY(EditAnywhere, Category="XPBD")
    int32 GridHeight = 32;

    UPROPERTY(EditAnywhere, Category="XPBD")
    float ParticleSpacing = 5.0f;

    UPROPERTY(EditAnywhere, Category="XPBD")
    uint32 IterationCount = 8;

private:
    void BuildInitialData();

    TArray<FVector4f> Positions;
    TArray<FVector4f> Velocities;
    TArray<FDistanceConstraintData> Constraints;
};
