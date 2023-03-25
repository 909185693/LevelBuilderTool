// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelBuilderToolEditor.h"
#include "LevelBuilderToolEdMode.h"
#include "EditorModeRegistry.h"

#define LOCTEXT_NAMESPACE "FLevelBuilderToolModule"

void FLevelBuilderToolEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FEditorModeRegistry::Get().RegisterMode<FLevelBuilderToolEdMode>(FLevelBuilderToolEdMode::EM_LevelBuilderToolEdModeId, LOCTEXT("LevelBuilderToolEdModeName", "LevelBuilderToolEdMode"), FSlateIcon(), true);
}

void FLevelBuilderToolEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FEditorModeRegistry::Get().UnregisterMode(FLevelBuilderToolEdMode::EM_LevelBuilderToolEdModeId);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLevelBuilderToolEditorModule, LevelBuilderToolEditor)