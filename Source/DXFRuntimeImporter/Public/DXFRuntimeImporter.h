// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "assimp/Logger.hpp"
#include "assimp/LogStream.hpp"

DECLARE_LOG_CATEGORY_EXTERN(LogAssimp, Log, All);

// Example stream
class UEAssimpStream : public Assimp::Logger
{
public:
	// Constructor
	UEAssimpStream()
	{
		// empty
	}
        
	// Destructor
	virtual ~UEAssimpStream() override
	{
		// empty
	}

	// Log to unreal output log
	virtual void OnError(const char* message) override
	{
		UE_LOG(LogAssimp,Error,TEXT("%s"),*FString(message));
	}
	virtual void OnDebug(const char* message) override
	{
		UE_LOG(LogAssimp,Log,TEXT("%s"),*FString(message));
	}
	virtual void OnInfo(const char* message) override
	{
		UE_LOG(LogAssimp,Log,TEXT("%s"),*FString(message));
	}
	virtual void OnWarn(const char* message) override
	{
		UE_LOG(LogAssimp,Warning,TEXT("%s"),*FString(message));
	}
	virtual void OnVerboseDebug(const char* message) override
	{
		UE_LOG(LogAssimp,Warning,TEXT("%s"),*FString(message));
	}
	virtual bool attachStream(Assimp::LogStream* pStream, unsigned severity) override
	{
		return false;
	}
	virtual bool detachStream(Assimp::LogStream* pStream, unsigned severity) override
	{
		return false;
	}
};

class FDXFRuntimeImporterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Handle to the test dll we will load */
};
