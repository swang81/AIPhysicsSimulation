#include "XPBDClothDemoActor.h"

#include "Components/SceneComponent.h"
#include "XPBDClothSimulationComponent.h"

AXPBDClothDemoActor::AXPBDClothDemoActor()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = SceneRoot;

    ClothSimulation = CreateDefaultSubobject<UXPBDClothSimulationComponent>(TEXT("XPBDClothSimulation"));
}
