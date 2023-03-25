// Copyright Epic Games, Inc. All Rights Reserved.

#include "LevelBuilderToolEdMode.h"
#include "LevelBuilderToolEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FLevelBuilderToolEdMode::EM_LevelBuilderToolEdModeId = TEXT("LevelBuilderToolTool");

FLevelBuilderToolEdMode::FLevelBuilderToolEdMode()
{

}

FLevelBuilderToolEdMode::~FLevelBuilderToolEdMode()
{

}

void FLevelBuilderToolEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FLevelBuilderToolEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FLevelBuilderToolEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	// Call base Exit method to ensure proper cleanup
	FEdMode::Exit();
}

bool FLevelBuilderToolEdMode::UsesToolkits() const
{
	return true;
}




