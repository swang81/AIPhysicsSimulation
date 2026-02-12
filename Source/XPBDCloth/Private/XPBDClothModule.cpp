#include "XPBDClothModule.h"

#include "Misc/Paths.h"
#include "ShaderCore.h"

void FXPBDClothModule::StartupModule()
{
    const FString ShaderDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/XPBDCloth"), ShaderDir);
}

void FXPBDClothModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FXPBDClothModule, XPBDCloth)
