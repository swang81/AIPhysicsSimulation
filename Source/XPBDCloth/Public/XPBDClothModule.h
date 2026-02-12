#pragma once

#include "Modules/ModuleInterface.h"

class FXPBDClothModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
