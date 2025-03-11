#pragma once

#include "Modules/ModuleManager.h"

class COMMONTRAIN_API FCommonTrainModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
