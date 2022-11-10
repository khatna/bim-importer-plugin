// Copyright Epic Games, Inc. All Rights Reserved.

#include "DXFRuntimeImporter.h"
#include "Core.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FDXFRuntimeImporterModule"

void FDXFRuntimeImporterModule::StartupModule()
{
	
}

void FDXFRuntimeImporterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDXFRuntimeImporterModule, DXFRuntimeImporter)
