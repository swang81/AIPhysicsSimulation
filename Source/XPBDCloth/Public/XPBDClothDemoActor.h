#pragma once

#include "GameFramework/Actor.h"
#include "XPBDClothDemoActor.generated.h"

class UXPBDClothSimulationComponent;

UCLASS()
class XPBDCLOTH_API AXPBDClothDemoActor : public AActor
{
    GENERATED_BODY()

public:
    AXPBDClothDemoActor();

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UXPBDClothSimulationComponent> ClothSimulation;
};
