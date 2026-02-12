using UnrealBuildTool;

public class AIPhysicsSimulation : ModuleRules
{
    public AIPhysicsSimulation(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "XPBDCloth"
        });
    }
}
